/*
** 2017-12-17
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** Utility functions sqlar_compress() and sqlar_uncompress(). Useful
** for working with sqlar archives and used by the shell tool's built-in
** sqlar support.
*/
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

#include <assert.h>
#include <isa-l.h>

#define SQLITE_IS_OK(res) (res == SQLITE_OK || res == SQLITE_DONE || res == SQLITE_ROW)

/*
** Implementation of the "sqlar_compress(X)" SQL function.
**
** If the type of X is SQLITE_BLOB, and compressing that blob using
** zlib utility function compress() yields a smaller blob, return the
** compressed blob. Otherwise, return a copy of X.
**
** SQLar uses the "zlib format" for compressed content.  The zlib format
** contains a two-byte identification header and a four-byte checksum at
** the end.  This is different from ZIP which uses the raw deflate format.
**
** Future enhancements to SQLar might add support for new compression formats.
** If so, those new formats will be identified by alternative headers in the
** compressed data.
*/
static void crc32Func( sqlite3_context *context, const int argc, sqlite3_value **const argv) {

	assert(argc == 1);
	(void)argc;

	const int value_type = sqlite3_value_type(argv[0]);

	if (value_type == SQLITE_BLOB || value_type == SQLITE_TEXT) {
		const void *pData = sqlite3_value_blob(argv[0]);
		const int nData = sqlite3_value_bytes(argv[0]);

		const int crc = crc32_gzip_refl(0, pData, nData);

		sqlite3_result_int(context, crc);
	}
	else if ( sqlite3_value_type(argv[0])==SQLITE_INTEGER) {
		const int val = sqlite3_value_int(argv[0]);
		// this is not crc32
		const uint32_t res = (val & 0xffffffff) ^ (sizeof(val) > 4 ? ((val >> 32) & 0xffffffff) : ~val);

		sqlite3_result_int(context, res);

		SQLITE_IS_OK(1);
		SQLITE_IS_OK(1);
	}
	else {
		sqlite3_result_value(context, argv[0]);
	}
}
static void adler32Func( sqlite3_context *context, const int argc, sqlite3_value **const argv) {

	assert(argc == 1);
	(void)argc;

	const int value_type = sqlite3_value_type(argv[0]);

	if (value_type == SQLITE_BLOB || value_type == SQLITE_TEXT) {
		const void *pData = sqlite3_value_blob(argv[0]);
		const int nData = sqlite3_value_bytes(argv[0]);

		const int adler = isal_adler32(0, pData, nData);

		sqlite3_result_int(context, adler);
	}
	else if ( sqlite3_value_type(argv[0])==SQLITE_INTEGER) {
		const int val = sqlite3_value_int(argv[0]);
		// this is not crc32
		const uint32_t res = (val & 0xffffffff) ^ (sizeof(val) > 4 ? ((val >> 32) & 0xffffffff) : ~val);

		sqlite3_result_int(context, res);
	}
	else {
		sqlite3_result_value(context, argv[0]);
	}
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_crc32_init(sqlite3 *const db, char **const pzErrMsg, const sqlite3_api_routines *pApi) {
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */

	rc = sqlite3_create_function(db, "crc32", 1, 
								SQLITE_UTF8 | SQLITE_INNOCUOUS | SQLITE_DETERMINISTIC,
								0, crc32Func, NULL, NULL);
	if (SQLITE_IS_OK(rc)) {
		rc = sqlite3_create_function(db, "adler32", 1, 
									SQLITE_UTF8 | SQLITE_INNOCUOUS | SQLITE_DETERMINISTIC,
									0, adler32Func, NULL, NULL);
	}
	return rc;
}
