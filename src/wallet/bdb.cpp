// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/bdb.h>
#include <wallet/db.h>

#include <util/strencodings.h>
#include <util/translation.h>

#include <stdint.h>

#ifndef WIN32
#include <sys/stat.h>
#endif

namespace {

//! Make sure database has a unique fileid within the environment. If it
//! doesn't, throw an error. BDB caches do not work properly when more than one
//! open database has the same fileid (values written to one database may show
//! up in reads to other databases).
//!
//! BerkeleyDB generates unique fileids by default
//! (https://docs.oracle.com/cd/E17275_01/html/programmer_reference/program_copy.html),
//! so bitcoin should never create different databases with the same fileid, but
//! this error can be triggered if users manually copy database files.
void CheckUniqueFileid(const BerkeleyEnvironment& env, const std::string& filename, Db& db, WalletDatabaseFileId& fileid)
{
    if (env.IsMock()) return;

    int ret = db.get_mpf()->get_fileid(fileid.value);
    if (ret != 0) {
        throw std::runtime_error(strprintf("BerkeleyBatch: Can't open database %s (get_fileid failed with %d)", filename, ret));
    }

    for (const auto& item : env.m_fileids) {
        if (fileid == item.second && &fileid != &item.second) {
            throw std::runtime_error(strprintf("BerkeleyBatch: Can't open database %s (duplicates fileid %s from %s)", filename,
            HexStr(item.second.value), item.first));
        }
    }
}

CCriticalSection cs_db;
std::map<std::string, std::weak_ptr<BerkeleyEnvironment>> g_dbenvs GUARDED_BY(cs_db); //!< Map from directory name to db environment.
} // namespace

bool WalletDatabaseFileId::operator==(const WalletDatabaseFileId& rhs) const
{
    return memcmp(value, &rhs.value, sizeof(value)) == 0;
}

bool IsBDBWalletLoaded(const fs::path& wallet_path)
{
    fs::path env_directory;
    std::string database_filename;
    SplitWalletPath(wallet_path, env_directory, database_filename);
    LOCK(cs_db);
    auto env = g_dbenvs.find(env_directory.string());
    if (env == g_dbenvs.end()) return false;
    auto database = env->second.lock();
    return database && database->IsDatabaseLoaded(database_filename);
}

/**
 * @param[in] wallet_path Path to wallet directory. Or (for backwards compatibility only) a path to a berkeley btree data file inside a wallet directory.
 * @param[out] database_filename Filename of berkeley btree data file inside the wallet directory.
 * @return A shared pointer to the BerkeleyEnvironment object for the wallet directory, never empty because ~BerkeleyEnvironment
 * erases the weak pointer from the g_dbenvs map.
 * @post A new BerkeleyEnvironment weak pointer is inserted into g_dbenvs if the directory path key was not already in the map.
 */
std::shared_ptr<BerkeleyEnvironment> GetWalletEnv(const fs::path& wallet_path, std::string& database_filename)
{
    fs::path env_directory;
    SplitWalletPath(wallet_path, env_directory, database_filename);
    LOCK(cs_db);
    auto inserted = g_dbenvs.emplace(env_directory.string(), std::weak_ptr<BerkeleyEnvironment>());
    if (inserted.second) {
        auto env = std::make_shared<BerkeleyEnvironment>(env_directory.string());
        inserted.first->second = env;
        return env;
    }
    return inserted.first->second.lock();
}

//
// BerkeleyBatch
//

void BerkeleyEnvironment::Close()
{
    if (!fDbEnvInit)
        return;

    fDbEnvInit = false;

    for (auto& db : m_databases) {
        auto count = mapFileUseCount.find(db.first);
        assert(count == mapFileUseCount.end() || count->second == 0);
        BerkeleyDatabase& database = db.second.get();
        if (database.m_db) {
            database.m_db->close(0);
            database.m_db.reset();
        }
    }

    FILE* error_file = nullptr;
    dbenv->get_errfile(&error_file);

    int ret = dbenv->close(0);
    if (ret != 0)
        LogPrintf("BerkeleyEnvironment::Close: Error %d closing database environment: %s\n", ret, DbEnv::strerror(ret));
    if (!fMockDb)
        DbEnv((u_int32_t)0).remove(strPath.c_str(), 0);

    if (error_file) fclose(error_file);

    UnlockDirectory(strPath, ".walletlock");
}

void BerkeleyEnvironment::Reset()
{
    dbenv.reset(new DbEnv(DB_CXX_NO_EXCEPTIONS));
    fDbEnvInit = false;
    fMockDb = false;
}

BerkeleyEnvironment::BerkeleyEnvironment(const fs::path& dir_path) : strPath(dir_path.string())
{
    Reset();
}

BerkeleyEnvironment::~BerkeleyEnvironment()
{
    LOCK(cs_db);
    g_dbenvs.erase(strPath);
    Close();
}

bool BerkeleyEnvironment::Open(bilingual_str& err)
{
    if (fDbEnvInit) {
        return true;
    }

    fs::path pathIn = strPath;
    TryCreateDirectories(pathIn);
    if (!LockDirectory(pathIn, ".walletlock")) {
        LogPrintf("Cannot obtain a lock on wallet directory %s. Another instance of SPRINGBOK Core may be using it.\n", strPath);
        err = strprintf(_("Error initializing wallet database environment %s!"), Directory());
        return false;
    }

    fs::path pathLogDir = pathIn / "database";
    TryCreateDirectories(pathLogDir);
    fs::path pathErrorFile = pathIn / "db.log";
    LogPrintf("BerkeleyEnvironment::Open: LogDir=%s ErrorFile=%s\n", pathLogDir.string(), pathErrorFile.string());

    unsigned int nEnvFlags = 0;
    if (gArgs.GetBoolArg("-privdb", DEFAULT_WALLET_PRIVDB))
        nEnvFlags |= DB_PRIVATE;

    dbenv->set_lg_dir(pathLogDir.string().c_str());
    dbenv->set_cachesize(0, 0x100000, 1); // 1 MiB should be enough for just the wallet
    dbenv->set_lg_bsize(0x10000);
    dbenv->set_lg_max(1048576);
    dbenv->set_lk_max_locks(40000);
    dbenv->set_lk_max_objects(40000);
    dbenv->set_errfile(fsbridge::fopen(pathErrorFile, "a")); /// debug
    dbenv->set_flags(DB_AUTO_COMMIT, 1);
    dbenv->set_flags(DB_TXN_WRITE_NOSYNC, 1);
    dbenv->log_set_config(DB_LOG_AUTO_REMOVE, 1);
    int ret = dbenv->open(strPath.c_str(),
                         DB_CREATE |
                             DB_INIT_LOCK |
                             DB_INIT_LOG |
                             DB_INIT_MPOOL |
                             DB_INIT_TXN |
                             DB_THREAD |
                             DB_RECOVER |
                             nEnvFlags,
                         S_IRUSR | S_IWUSR);
    if (ret != 0) {
        LogPrintf("BerkeleyEnvironment::Open: Error %d opening database environment: %s\n", ret, DbEnv::strerror(ret));
        int ret2 = dbenv->close(0);
        if (ret2 != 0) {
            LogPrintf("BerkeleyEnvironment::Open: Error %d closing failed database environment: %s\n", ret2, DbEnv::strerror(ret2));
        }
        Reset();
        err = strprintf(_("Error initializing wallet database environment %s!"), Directory());
        if (ret == DB_RUNRECOVERY) {
            err += Untranslated(" ") + _("This error could occur if this wallet was not shutdown cleanly and was last loaded using a build with a newer version of Berkeley DB. If so, please use the software that last loaded this wallet");
        }
        return false;
    }

    fDbEnvInit = true;
    fMockDb = false;
    return true;
}

//! Construct an in-memory mock Berkeley environment for testing
BerkeleyEnvironment::BerkeleyEnvironment()
{
    Reset();

    LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::MakeMock\n");

    dbenv->set_cachesize(1, 0, 1);
    dbenv->set_lg_bsize(10485760 * 4);
    dbenv->set_lg_max(10485760);
    dbenv->set_lk_max_locks(10000);
    dbenv->set_lk_max_objects(10000);
    dbenv->set_flags(DB_AUTO_COMMIT, 1);
    dbenv->log_set_config(DB_LOG_IN_MEMORY, 1);
    int ret = dbenv->open(nullptr,
                         DB_CREATE |
                             DB_INIT_LOCK |
                             DB_INIT_LOG |
                             DB_INIT_MPOOL |
                             DB_INIT_TXN |
                             DB_THREAD |
                             DB_PRIVATE,
                         S_IRUSR | S_IWUSR);
    if (ret > 0) {
        throw std::runtime_error(strprintf("BerkeleyEnvironment::MakeMock: Error %d opening database environment.", ret));
    }

    fDbEnvInit = true;
    fMockDb = true;
}

bool BerkeleyEnvironment::Verify(const std::string& strFile)
{
    LOCK(cs_db);
    assert(mapFileUseCount.count(strFile) == 0);

    Db db(dbenv.get(), 0);
    int result = db.verify(strFile.c_str(), nullptr, nullptr, 0);
    return result == 0;
}

BerkeleyBatch::SafeDbt::SafeDbt()
{
    m_dbt.set_flags(DB_DBT_MALLOC);
}

BerkeleyBatch::SafeDbt::SafeDbt(void* data, size_t size)
    : m_dbt(data, size)
{
}

BerkeleyBatch::SafeDbt::~SafeDbt()
{
    if (m_dbt.get_data() != nullptr) {
        // Clear memory, e.g. in case it was a private key
        memory_cleanse(m_dbt.get_data(), m_dbt.get_size());
        // under DB_DBT_MALLOC, data is malloced by the Dbt, but must be
        // freed by the caller.
        // https://docs.oracle.com/cd/E17275_01/html/api_reference/C/dbt.html
        if (m_dbt.get_flags() & DB_DBT_MALLOC) {
            free(m_dbt.get_data());
        }
    }
}

const void* BerkeleyBatch::SafeDbt::get_data() const
{
    return m_dbt.get_data();
}

u_int32_t BerkeleyBatch::SafeDbt::get_size() const
{
    return m_dbt.get_size();
}

BerkeleyBatch::SafeDbt::operator Dbt*()
{
    return &m_dbt;
}

bool BerkeleyDatabase::Verify(bilingual_str& errorStr)
{
    fs::path walletDir = env->Directory();
    fs::path file_path = walletDir / strFile;

    LogPrintf("Using BerkeleyDB version %s\n", BerkeleyDatabaseVersion());
    LogPrintf("Using wallet %s\n", file_path.string());

    if (!env->Open(errorStr)) {
        return false;
    }

    if (fs::exists(file_path))
    {
        if (!env->Verify(strFile)) {
            errorStr = strprintf(_("%s corrupt. Try using the wallet tool dash-wallet to salvage or restoring a backup."), file_path);
            return false;
        }
    }
    // also return true if files does not exists
    return true;
}

void BerkeleyEnvironment::CheckpointLSN(const std::string& strFile)
{
    dbenv->txn_checkpoint(0, 0, 0);
    if (fMockDb)
        return;
    dbenv->lsn_reset(strFile.c_str(), 0);
}

BerkeleyDatabase::~BerkeleyDatabase()
{
    if (env) {
        LOCK(cs_db);
        size_t erased = env->m_databases.erase(strFile);
        assert(erased == 1);
        env->m_fileids.erase(strFile);
    }
}

BerkeleyBatch::BerkeleyBatch(BerkeleyDatabase& database, const char* pszMode, bool fFlushOnCloseIn) : pdb(nullptr), activeTxn(nullptr), m_cursor(nullptr), m_database(database)
{
    fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
    fFlushOnClose = fFlushOnCloseIn;
    env = database.env.get();
    const std::string &strFilename = database.strFile;

    bool fCreate = strchr(pszMode, 'c') != nullptr;
    unsigned int nFlags = DB_THREAD;
    if (fCreate)
        nFlags |= DB_CREATE;

    {
        LOCK(cs_db);
        bilingual_str open_err;
        if (!env->Open(open_err))
            throw std::runtime_error("BerkeleyBatch: Failed to open database environment.");

        pdb = database.m_db.get();
        if (pdb == nullptr) {
            int ret;
            std::unique_ptr<Db> pdb_temp = MakeUnique<Db>(env->dbenv.get(), 0);

            bool fMockDb = env->IsMock();
            if (fMockDb) {
                DbMpoolFile* mpf = pdb_temp->get_mpf();
                ret = mpf->set_flags(DB_MPOOL_NOFILE, 1);
                if (ret != 0) {
                    throw std::runtime_error(strprintf("BerkeleyBatch: Failed to configure for no temp file backing for database %s", strFilename));
                }
            }

            ret = pdb_temp->open(nullptr,                             // Txn pointer
                            fMockDb ? nullptr : strFilename.c_str(),  // Filename
                            fMockDb ? strFilename.c_str() : "main",   // Logical db name
                            DB_BTREE,                                 // Database type
                            nFlags,                                   // Flags
                            0);

            if (ret != 0) {
                throw std::runtime_error(strprintf("BerkeleyBatch: Error %d, can't open database %s", ret, strFilename));
            }

            // Call CheckUniqueFileid on the containing BDB environment to
            // avoid BDB data consistency bugs that happen when different data
            // files in the same environment have the same fileid.
            //
            // Also call CheckUniqueFileid on all the other g_dbenvs to prevent
            // bitcoin from opening the same data file through another
            // environment when the file is referenced through equivalent but
            // not obviously identical symlinked or hard linked or bind mounted
            // paths. In the future a more relaxed check for equal inode and
            // device ids could be done instead, which would allow opening
            // different backup copies of a wallet at the same time. Maybe even
            // more ideally, an exclusive lock for accessing the database could
            // be implemented, so no equality checks are needed at all. (Newer
            // versions of BDB have an set_lk_exclusive method for this
            // purpose, but the older version we use does not.)
            for (const auto& env : g_dbenvs) {
                CheckUniqueFileid(*env.second.lock().get(), strFilename, *pdb_temp, this->env->m_fileids[strFilename]);
            }

            pdb = pdb_temp.release();
            database.m_db.reset(pdb);

            if (fCreate && !Exists(std::string("version"))) {
                bool fTmp = fReadOnly;
                fReadOnly = false;
                Write(std::string("version"), CLIENT_VERSION);
                fReadOnly = fTmp;
            }
        }
        database.AddRef();
        strFile = strFilename;
    }
}

void BerkeleyDatabase::Open(const char* mode)
{
    throw std::logic_error("BerkeleyDatabase does not implement Open. This function should not be called.");
}

void BerkeleyBatch::Flush()
{
    if (activeTxn)
        return;

    // Flush database activity from memory pool to disk log
    unsigned int nMinutes = 0;
    if (fReadOnly)
        nMinutes = 1;

    if (env) { // env is nullptr for dummy databases (i.e. in tests). Don't actually flush if env is nullptr so we don't segfault
        env->dbenv->txn_checkpoint(nMinutes ? gArgs.GetArg("-dblogsize", DEFAULT_WALLET_DBLOGSIZE) * 1024 : 0, nMinutes, 0);
    }
}

void BerkeleyDatabase::IncrementUpdateCounter()
{
    ++nUpdateCounter;
}

void BerkeleyBatch::Close()
{
    if (!pdb)
        return;
    if (activeTxn)
        activeTxn->abort();
    activeTxn = nullptr;
    pdb = nullptr;
    CloseCursor();

    if (fFlushOnClose)
        Flush();

    m_database.RemoveRef();
}

void BerkeleyEnvironment::CloseDb(const std::string& strFile)
{
    {
        LOCK(cs_db);
        auto it = m_databases.find(strFile);
        assert(it != m_databases.end());
        BerkeleyDatabase& database = it->second.get();
        if (database.m_db) {
            // Close the database handle
            database.m_db->close(0);
            database.m_db.reset();
        }
    }
}

void BerkeleyEnvironment::ReloadDbEnv()
{
    // Make sure that no Db's are in use
    AssertLockNotHeld(cs_db);
    std::unique_lock<CCriticalSection> lock(cs_db);
    m_db_in_use.wait(lock, [this](){
        for (auto& count : mapFileUseCount) {
            if (count.second > 0) return false;
        }
        return true;
    });

    std::vector<std::string> filenames;
    for (auto it : m_databases) {
        filenames.push_back(it.first);
    }
    // Close the individual Db's
    for (const std::string& filename : filenames) {
        CloseDb(filename);
    }
    // Reset the environment
    Flush(true); // This will flush and close the environment
    Reset();
    bilingual_str open_err;
    Open(open_err);
}

bool BerkeleyDatabase::Rewrite(const char* pszSkip)
{
    while (true) {
        {
            LOCK(cs_db);
            if (!env->mapFileUseCount.count(strFile) || env->mapFileUseCount[strFile] == 0) {
                // Flush log data to the dat file
                env->CloseDb(strFile);
                env->CheckpointLSN(strFile);
                env->mapFileUseCount.erase(strFile);

                bool fSuccess = true;
                LogPrintf("BerkeleyBatch::Rewrite: Rewriting %s...\n", strFile);
                std::string strFileRes = strFile + ".rewrite";
                { // surround usage of db with extra {}
                    BerkeleyBatch db(*this, "r");
                    std::unique_ptr<Db> pdbCopy = MakeUnique<Db>(env->dbenv.get(), 0);

                    int ret = pdbCopy->open(nullptr,               // Txn pointer
                                            strFileRes.c_str(), // Filename
                                            "main",             // Logical db name
                                            DB_BTREE,           // Database type
                                            DB_CREATE,          // Flags
                                            0);
                    if (ret > 0) {
                        LogPrintf("BerkeleyBatch::Rewrite: Can't create database file %s\n", strFileRes);
                        fSuccess = false;
                    }

                    if (db.StartCursor()) {
                        while (fSuccess) {
                            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
                            CDataStream ssValue(SER_DISK, CLIENT_VERSION);
                            bool complete;
                            bool ret1 = db.ReadAtCursor(ssKey, ssValue, complete);
                            if (complete) {
                                break;
                            } else if (!ret1) {
                                fSuccess = false;
                                break;
                            }
                            if (pszSkip &&
                                strncmp(ssKey.data(), pszSkip, std::min(ssKey.size(), strlen(pszSkip))) == 0)
                                continue;
                            if (strncmp(ssKey.data(), "\x07version", 8) == 0) {
                                // Update version:
                                ssValue.clear();
                                ssValue << CLIENT_VERSION;
                            }
                            Dbt datKey(ssKey.data(), ssKey.size());
                            Dbt datValue(ssValue.data(), ssValue.size());
                            int ret2 = pdbCopy->put(nullptr, &datKey, &datValue, DB_NOOVERWRITE);
                            if (ret2 > 0)
                                fSuccess = false;
                        }
                        db.CloseCursor();
                    }
                    if (fSuccess) {
                        db.Close();
                        env->CloseDb(strFile);
                        if (pdbCopy->close(0))
                            fSuccess = false;
                    } else {
                        pdbCopy->close(0);
                    }
                }
                if (fSuccess) {
                    Db dbA(env->dbenv.get(), 0);
                    if (dbA.remove(strFile.c_str(), nullptr, 0))
                        fSuccess = false;
                    Db dbB(env->dbenv.get(), 0);
                    if (dbB.rename(strFileRes.c_str(), nullptr, strFile.c_str(), 0))
                        fSuccess = false;
                }
                if (!fSuccess)
                    LogPrintf("BerkeleyBatch::Rewrite: Failed to rewrite database file %s\n", strFileRes);
                return fSuccess;
            }
        }
        UninterruptibleSleep(std::chrono::milliseconds{100});
    }
}


void BerkeleyEnvironment::Flush(bool fShutdown)
{
    int64_t nStart = GetTimeMillis();
    // Flush log data to the actual data file on all files that are not in use
    LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::Flush: [%s] Flush(%s)%s\n", strPath, fShutdown ? "true" : "false", fDbEnvInit ? "" : " database not started");
    if (!fDbEnvInit)
        return;
    {
        LOCK(cs_db);
        std::map<std::string, int>::iterator mi = mapFileUseCount.begin();
        while (mi != mapFileUseCount.end()) {
            std::string strFile = (*mi).first;
            int nRefCount = (*mi).second;
            LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::Flush: Flushing %s (refcount = %d)...\n", strFile, nRefCount);
            if (nRefCount == 0) {
                // Move log data to the dat file
                CloseDb(strFile);
                LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::Flush: %s checkpoint\n", strFile);
                dbenv->txn_checkpoint(0, 0, 0);
                LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::Flush: %s detach\n", strFile);
                if (!fMockDb)
                    dbenv->lsn_reset(strFile.c_str(), 0);
                LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::Flush: %s closed\n", strFile);
                mapFileUseCount.erase(mi++);
            } else
                mi++;
        }
        LogPrint(BCLog::WALLETDB, "BerkeleyEnvironment::Flush: Flush(%s)%s took %15dms\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " database not started", GetTimeMillis() - nStart);
        if (fShutdown) {
            char** listp;
            if (mapFileUseCount.empty()) {
                dbenv->log_archive(&listp, DB_ARCH_REMOVE);
                Close();
                if (!fMockDb) {
                    fs::remove_all(fs::path(strPath) / "database");
                }
            }
        }
    }
}

bool BerkeleyDatabase::PeriodicFlush()
{
    bool ret = false;
    TRY_LOCK(cs_db, lockDb);
    if (lockDb)
    {
        // Don't do this if any databases are in use
        int nRefCount = 0;
        std::map<std::string, int>::iterator mit = env->mapFileUseCount.begin();
        while (mit != env->mapFileUseCount.end())
        {
            nRefCount += (*mit).second;
            mit++;
        }

        if (nRefCount == 0)
        {
            std::map<std::string, int>::iterator mi = env->mapFileUseCount.find(strFile);
            if (mi != env->mapFileUseCount.end())
            {
                LogPrint(BCLog::WALLETDB, "Flushing %s\n", strFile);
                int64_t nStart = GetTimeMillis();

                // Flush wallet file so it's self contained
                env->CloseDb(strFile);
                env->CheckpointLSN(strFile);

                env->mapFileUseCount.erase(mi++);
                LogPrint(BCLog::WALLETDB, "Flushed %s %dms\n", strFile, GetTimeMillis() - nStart);
                ret = true;
            }
        }
    }

    return ret;
}

bool BerkeleyDatabase::Backup(const std::string& strDest) const
{
    while (true)
    {
        {
            LOCK(cs_db);
            if (!env->mapFileUseCount.count(strFile) || env->mapFileUseCount[strFile] == 0)
            {
                // Flush log data to the dat file
                env->CloseDb(strFile);
                env->CheckpointLSN(strFile);
                env->mapFileUseCount.erase(strFile);

                // Copy wallet file
                fs::path pathSrc = env->Directory() / strFile;
                fs::path pathDest(strDest);
                if (fs::is_directory(pathDest))
                    pathDest /= strFile;

                try {
                    if (fs::equivalent(pathSrc, pathDest)) {
                        LogPrintf("cannot backup to wallet source file %s\n", pathDest.string());
                        return false;
                    }

                    fs::copy_file(pathSrc, pathDest, fs::copy_option::overwrite_if_exists);
                    LogPrintf("copied %s to %s\n", strFile, pathDest.string());
                    return true;
                } catch (const fs::filesystem_error& e) {
                    LogPrintf("error copying %s to %s - %s\n", strFile, pathDest.string(), fsbridge::get_filesystem_error_message(e));
                    return false;
                }
            }
        }
        UninterruptibleSleep(std::chrono::milliseconds{100});
    }
}

void BerkeleyDatabase::Flush()
{
    env->Flush(false);
}

void BerkeleyDatabase::Close()
{
    env->Flush(true);
}

void BerkeleyDatabase::ReloadDbEnv()
{
    env->ReloadDbEnv();
}

bool BerkeleyBatch::StartCursor()
{
    assert(!m_cursor);
    if (!pdb)
        return false;
    int ret = pdb->cursor(nullptr, &m_cursor, 0);
    return ret == 0;
}

bool BerkeleyBatch::ReadAtCursor(CDataStream& ssKey, CDataStream& ssValue, bool& complete)
{
    complete = false;
    if (m_cursor == nullptr) return false;
    // Read at cursor
    SafeDbt datKey;
    SafeDbt datValue;
    int ret = m_cursor->get(datKey, datValue, DB_NEXT);
    if (ret == DB_NOTFOUND) {
        complete = true;
    }
    if (ret != 0)
        return false;
    else if (datKey.get_data() == nullptr || datValue.get_data() == nullptr)
        return false;

    // Convert to streams
    ssKey.SetType(SER_DISK);
    ssKey.clear();
    ssKey.write((char*)datKey.get_data(), datKey.get_size());
    ssValue.SetType(SER_DISK);
    ssValue.clear();
    ssValue.write((char*)datValue.get_data(), datValue.get_size());
    return true;
}

void BerkeleyBatch::CloseCursor()
{
    if (!m_cursor) return;
    m_cursor->close();
    m_cursor = nullptr;
}

bool BerkeleyBatch::TxnBegin()
{
    if (!pdb || activeTxn)
        return false;
    DbTxn* ptxn = env->TxnBegin();
    if (!ptxn)
        return false;
    activeTxn = ptxn;
    return true;
}

bool BerkeleyBatch::TxnCommit()
{
    if (!pdb || !activeTxn)
        return false;
    int ret = activeTxn->commit(0);
    activeTxn = nullptr;
    return (ret == 0);
}

bool BerkeleyBatch::TxnAbort()
{
    if (!pdb || !activeTxn)
        return false;
    int ret = activeTxn->abort();
    activeTxn = nullptr;
    return (ret == 0);
}

std::string BerkeleyDatabaseVersion()
{
    return DbEnv::version(nullptr, nullptr, nullptr);
}

bool BerkeleyBatch::ReadKey(CDataStream&& key, CDataStream& value)
{
    if (!pdb)
        return false;

    SafeDbt datKey(key.data(), key.size());

    SafeDbt datValue;
    int ret = pdb->get(activeTxn, datKey, datValue, 0);
    if (ret == 0 && datValue.get_data() != nullptr) {
        value.write((char*)datValue.get_data(), datValue.get_size());
        return true;
    }
    return false;
}

bool BerkeleyBatch::WriteKey(CDataStream&& key, CDataStream&& value, bool overwrite)
{
    if (!pdb)
        return false;
    if (fReadOnly)
        assert(!"Write called on database in read-only mode");

    SafeDbt datKey(key.data(), key.size());

    SafeDbt datValue(value.data(), value.size());

    int ret = pdb->put(activeTxn, datKey, datValue, (overwrite ? 0 : DB_NOOVERWRITE));
    return (ret == 0);
}

bool BerkeleyBatch::EraseKey(CDataStream&& key)
{
    if (!pdb)
        return false;
    if (fReadOnly)
        assert(!"Erase called on database in read-only mode");

    SafeDbt datKey(key.data(), key.size());

    int ret = pdb->del(activeTxn, datKey, 0);
    return (ret == 0 || ret == DB_NOTFOUND);
}

bool BerkeleyBatch::HasKey(CDataStream&& key)
{
    if (!pdb)
        return false;

    SafeDbt datKey(key.data(), key.size());

    int ret = pdb->exists(activeTxn, datKey, 0);
    return ret == 0;
}

void BerkeleyDatabase::AddRef()
{
    LOCK(cs_db);
    ++env->mapFileUseCount[strFile];
}

void BerkeleyDatabase::RemoveRef()
{
    {
        LOCK(cs_db);
        --env->mapFileUseCount[strFile];
    }
    env->m_db_in_use.notify_all();
}

std::unique_ptr<DatabaseBatch> BerkeleyDatabase::MakeBatch(const char* mode, bool flush_on_close)
{
    return MakeUnique<BerkeleyBatch>(*this, mode, flush_on_close);
}
