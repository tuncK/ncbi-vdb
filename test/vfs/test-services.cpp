/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ==============================================================================
*
* tests of names service
*/

#include <kfg/kfg-priv.h> /* KConfigMakeLocal */

#include <kfs/directory.h> /* KDirectoryRelease */
#include <kfs/file.h> /* KFileRelease */

#include <klib/debug.h> /* KDbgSetString */

#include <ktst/unit_test.hpp> /* KMain */

#include <vfs/manager.h> /* VFSManagerRelease */
#include <vfs/manager-priv.h> /* VFSManagerMakeLocal */
#include <vfs/path.h> /* VPathRelease */
#include <vfs/services-priv.h> /* KServiceMakeWithMgr */

#include <climits> /* PATH_MAX */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

using std::string;

TEST_SUITE ( TestServices );

#define ALL

#ifdef ALL
TEST_CASE ( TestKServiceAddId ) {
    KService * s = NULL;

    REQUIRE_RC_FAIL ( KServiceAddId ( s, "0" ) );

    REQUIRE_RC ( KServiceMake ( & s ) );

    REQUIRE_RC_FAIL ( KServiceAddId ( s, NULL ) );
    REQUIRE_RC_FAIL ( KServiceAddId ( s, "" ) );

    for ( int i = 0; i < 512; ++ i )
        REQUIRE_RC ( KServiceAddId ( s, "0" ) );
    REQUIRE_RC ( KServiceAddId ( s, "0" ) );
    for ( int i = 0; i < 512; ++ i )
        REQUIRE_RC ( KServiceAddId ( s, "0" ) );

    REQUIRE_RC ( KServiceRelease ( s ) );
}
#endif

#ifdef ALL
TEST_CASE(TestKSrvResponseGetLocation) {

#define ACC "SRR850901"
    
    KService * s = NULL;
    REQUIRE_RC(KServiceMake(&s));
    REQUIRE_RC(KServiceResolve(s, false, true));
    REQUIRE_RC(KServiceAddId(s, ACC));
    REQUIRE_RC(KServiceSetFormat(s, "all"));
    const KSrvResponse * r = NULL;
    REQUIRE_RC(KServiceNamesQuery(s, 0, &r));
    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(0, 0, 0, 0, 0, 0, 0, 0));
    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(r, 0, 0, 0, 0, 0, 0, 0));
    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(r, ACC, 0, 0, 0, 0, 0, 0));
    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(r, 0, ACC, 0, 0, 0, 0, 0));
    REQUIRE_RC(KSrvResponseGetLocation2(r, ACC, ACC , "sra",
        0, 0, 0, 0));

    const VPath * local = NULL;
    REQUIRE_RC(KSrvResponseGetLocation2(r, ACC, ACC , "sra",
        &local, 0, 0, 0));
    REQUIRE_NULL(local);

    rc_t rcLocal = 0;
    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(r, ACC, ACC , "sra",
        0, &rcLocal, 0, 0));
    REQUIRE_RC(KSrvResponseGetLocation2(r, ACC, ACC , "sra",
        &local, &rcLocal, 0, 0));
    REQUIRE_NULL(local);
    REQUIRE_RC_FAIL(rcLocal);

    const VPath * cache = NULL;
    rc_t rcCache = 0;

    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(r, ACC, ACC ".qq",
        "sra", &local, &rcLocal, &cache, &rcCache));
    REQUIRE_NULL(local);
    REQUIRE_NULL(cache);
    REQUIRE_RC(rcLocal);
    REQUIRE_RC(rcCache);

    REQUIRE_RC_FAIL(KSrvResponseGetLocation2(r, "SRR000001", "SRR000001", "sra",
        &local, &rcLocal, &cache, &rcCache));
    REQUIRE_NULL(local);
    REQUIRE_NULL(cache);
    REQUIRE_RC(rcLocal);
    REQUIRE_RC(rcCache);

    REQUIRE_RC(KSrvResponseRelease(r));
    REQUIRE_RC(KServiceRelease(s));
}
#endif

#ifdef ALL
TEST_CASE(TestKSrvResponseGetLocationCache) {
    KService * s = NULL;
    REQUIRE_RC(KServiceMake(&s));
    REQUIRE_RC(KServiceAddId(s, "SRR850901"));

    const KSrvResponse * r = NULL;
    REQUIRE_RC(KServiceNamesQuery(s, 0, &r));

    const VPath * local = NULL;
    rc_t rcLocal = 0;
    const VPath * cache = NULL;
    rc_t rcCache = 0;

    REQUIRE_RC(KSrvResponseGetLocation2(r, "SRR850901", "SRR850901",
        "sra", &local, &rcLocal, &cache, &rcCache));

	REQUIRE_RC_FAIL(rcLocal);
    
    char buffer[PATH_MAX] = "";

	REQUIRE_RC_FAIL(VPathReadPath(local, buffer, sizeof buffer, NULL));
    
    REQUIRE_RC(VPathRelease(local));

    REQUIRE_RC_FAIL(rcCache);
    REQUIRE_NULL(cache);

    REQUIRE_RC(KSrvResponseRelease(r));

    REQUIRE_RC(KServiceRelease(s));
}
#endif

#ifdef ALL
TEST_CASE(TestKSrvResponseGetLocationLocalInAD) {
    KDirectory * dr = NULL;
    REQUIRE_RC(KDirectoryNativeDir(&dr));
    const char * acc = "SRR850901";
    char p[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p, "%s", acc));
    REQUIRE_RC(KDirectoryCreateDir(dr, 0775, kcmOpen | kcmInit | kcmCreate, p));

    KFile * f = NULL;
    REQUIRE_RC(KDirectoryCreateFile(dr, &f, false,
        0664, kcmOpen | kcmInit | kcmCreate, "%s/%s.sra", p, acc));
    REQUIRE_RC(KFileRelease(f));

    KService * s = NULL;
    REQUIRE_RC(KServiceMakeWithMgr(&s, NULL, NULL, NULL));
    REQUIRE_RC(KServiceResolve(s, false, true));
    REQUIRE_RC(KServiceAddId(s, acc));

    const KSrvResponse * r = NULL;
    REQUIRE_RC(KServiceNamesQuery(s, 0, &r));

    const VPath * local = NULL;
    rc_t rcLocal = 0;
    const VPath * cache = NULL;
    rc_t rcCache = 0;

    REQUIRE_RC(KSrvResponseGetLocation2(r, acc, "SRR850901", "sra",
        &local, &rcLocal, &cache, &rcCache));

    REQUIRE_RC(rcLocal);
    REQUIRE_NOT_NULL(local);
    char u[PATH_MAX] = "";
    REQUIRE_RC(VPathReadPath(local, u, sizeof u, 0));
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p,
        "%s/%s.sra", acc, acc));
    REQUIRE_EQ(string(p), string(u));
    REQUIRE_RC(VPathRelease(local));

    REQUIRE_NULL(cache);
    REQUIRE_RC_FAIL(rcCache);

    REQUIRE_RC(KSrvResponseRelease(r));

    REQUIRE_RC(KServiceRelease(s));

    REQUIRE_RC(KDirectoryRemove(dr, true, acc));
    REQUIRE_RC(KDirectoryRelease(dr));
}
#endif

#ifdef ALL
TEST_CASE(TestKSrvResponseGetLocationLocalInUserRepo) {

#define ACC "SRR850901"

    KDirectory * dr = NULL;
    REQUIRE_RC(KDirectoryNativeDir(&dr));
    char p[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p, "tmp"));
    REQUIRE_RC(KDirectoryCreateDir(dr, 0775,
        kcmOpen | kcmInit | kcmCreate | kcmParents, "tmp/sra"));
    KFile * f = NULL;
    REQUIRE_RC(KDirectoryCreateFile(dr, &f, false,
        0664, kcmOpen | kcmInit | kcmCreate, "tmp/sra/%s.sra", ACC));
    REQUIRE_RC(KFileRelease(f));
    KConfig * kfg = NULL;
    REQUIRE_RC(KConfigMakeLocal(&kfg, NULL));
    REQUIRE_RC(KConfigWriteString(kfg, "/repository/user/main/public/root", p));
    REQUIRE_RC(KConfigWriteString(kfg,
        "/repository/user/main/public/apps/sra/volumes/sraFlat", "sra"));
//KConfigPrint(kfg,0);
    VFSManager * mgr = NULL;
    REQUIRE_RC(VFSManagerMakeLocal(&mgr, kfg));
    KService * s = NULL;
    REQUIRE_RC(KServiceMakeWithMgr(&s, mgr, NULL, kfg));
    REQUIRE_RC(KServiceResolve(s, false, true));
    REQUIRE_RC(KServiceAddId(s, ACC));
    const KSrvResponse * r = NULL;
    REQUIRE_RC(KServiceNamesQuery(s, 0, &r));
    const VPath * local = NULL;
    rc_t rcLocal = 0;
    const VPath * cache = NULL;
    rc_t rcCache = 0;
    REQUIRE_RC(KSrvResponseGetLocation2(r, ACC, ACC , "sra",
        &local, &rcLocal, &cache, &rcCache));
    REQUIRE_RC(rcLocal);
    REQUIRE_NOT_NULL(local);
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p,
        "tmp/sra/%s.sra", ACC));
    char u[PATH_MAX] = "";
    REQUIRE_RC(VPathReadPath(local, u, sizeof u, 0));
    REQUIRE_EQ(string(p), string(u));
    REQUIRE_RC(VPathRelease(local));
    REQUIRE_RC(rcCache);
    REQUIRE_NOT_NULL(cache);
    REQUIRE_RC(VPathReadPath(cache, u, sizeof u, 0));
    REQUIRE_EQ(string(p), string(u));
    REQUIRE_RC(VPathRelease(cache));
    REQUIRE_RC(KSrvResponseRelease(r));
    REQUIRE_RC(KServiceRelease(s));
    REQUIRE_RC(VFSManagerRelease(mgr));
    REQUIRE_RC(KConfigRelease(kfg));
    REQUIRE_RC(KDirectoryRemove(dr, true, "tmp"));
    REQUIRE_RC(KDirectoryRelease(dr));
}
#endif

#ifdef ALL
TEST_CASE(TestKSrvResponseGetLocationCacheInAD) {

#define ACC "SRR850901"

    KDirectory * dr = NULL;
    REQUIRE_RC(KDirectoryNativeDir(&dr));
    char p[PATH_MAX] = "";
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p, "%s", ACC));
    REQUIRE_RC(KDirectoryCreateDir(dr, 0775, kcmOpen | kcmInit | kcmCreate, p));

    KFile * f = NULL;
    REQUIRE_RC(KDirectoryCreateFile(dr, &f, false,
        0664, kcmOpen | kcmInit | kcmCreate, "%s/%s.sra", p, ACC));
    REQUIRE_RC(KFileRelease(f));

    VFSManager * mgr = NULL;
    REQUIRE_RC(VFSManagerMakeLocal(&mgr, NULL));

    /* the same call is done by prefetch to enable caching to AD */
    REQUIRE_RC(VFSManagerSetAdCaching(mgr, true));

    KService * s = NULL;
    REQUIRE_RC(KServiceMakeWithMgr(&s, mgr, NULL, NULL));
    REQUIRE_RC(KServiceResolve(s, false, true));
    REQUIRE_RC(KServiceAddId(s, ACC));

    const KSrvResponse * r = NULL;
    REQUIRE_RC(KServiceNamesQuery(s, 0, &r));

    const VPath * local = NULL;
    rc_t rcLocal = 0;
    const VPath * cache = NULL;
    rc_t rcCache = 0;

    REQUIRE_RC(KSrvResponseGetLocation2(r, ACC, ACC , "sra",
        &local, &rcLocal, &cache, &rcCache));

    REQUIRE_RC(rcLocal);
    REQUIRE_NOT_NULL(local);
    char u[PATH_MAX] = "";
    REQUIRE_RC(VPathReadPath(local, u, sizeof u, 0));
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p,
        "%s/%s.sra", ACC, ACC));
    REQUIRE_EQ(string(p), string(u));
    REQUIRE_RC(VPathRelease(local));

    REQUIRE_RC(rcCache);
    REQUIRE_NOT_NULL(cache);
    REQUIRE_RC(KDirectoryResolvePath(dr, true, p, sizeof p,
        "%s/%s.sra", ACC, ACC));
    REQUIRE_EQ(string(p), string(u));
    REQUIRE_RC(VPathRelease(cache));

    REQUIRE_RC(KSrvResponseRelease(r));

    REQUIRE_RC(KServiceRelease(s));

    REQUIRE_RC(VFSManagerRelease(mgr));

    REQUIRE_RC(KDirectoryRemove(dr, true, ACC));
    REQUIRE_RC(KDirectoryRelease(dr));
}
#endif

#ifndef ALL
TEST_CASE(TestKSrvResponseGetLocationCacheInADLeak) {
}
#endif

extern "C" {
    ver_t CC KAppVersion ( void ) { return 0; }

    rc_t CC KMain ( int argc, char * argv [] ) {
        KConfigDisableUserSettings(); // ignore ~/.ncbi/user-settings.mkfg

        if (
0) assert(!KDbgSetString("VFS"));

        putenv ( "NCBI_VDB_QUALITY=R" );
        return TestServices ( argc, argv );
    }
}
