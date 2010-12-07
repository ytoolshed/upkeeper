#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <store/upk_db.h>

int DEBUG = 0;

/*
 *
 */
void usage(
    char *prog,
    char *message
) {
    printf( "%s\n", message );
    printf( "usage: %s sqlite.db\n", prog );
    exit( 1 );
}

/*
 *
 */
int main( 
    int   argc, 
    char *argv[] 
) {
#ifdef UNDER_CONSTRUCTION
    char *file        = NULL;
    char *file_backup = NULL;
    char *file_dump   = NULL;
    int   rc          = 0;
    char *command     = NULL;
    FILE *pipe        = NULL;
    FILE *fpdump      = NULL;
    char buf[1024];

    if( argc != 2 ) {
	usage(argv[0], "Not enough arguments");
	return( 1 );
    }

    file = argv[1];

    command = sqlite3_mprintf( "sqlite3 %s .dump", file);

    if( (pipe = popen( command, "r" )) == NULL ) {
	printf( "Opening pipe to %s failed: %s\n",
		command, strerror( errno ) );
	goto cleanup;
    }

    file_dump   = sqlite3_mprintf( "%s.dump", file );

    if( (fpdump = fopen( file_dump, "w" )) == NULL ) {
	printf( "Opening %s failed: %s\n", file_dump, strerror(errno) );
	rc = -2;
	goto cleanup;
    }

    while( fgets( pipe, buf, sizeof( buf ) ) ) {
	fprintf(
    }

    file_backup = sqlite3_mprintf( "%s.bak", file );

    if( DEBUG ) {
	printf( "Renaming %s to %s\n", file, file_backup );
    }

    rc = rename( file, file_backup );

    if( rc != 0 ) {
	printf( "Renaming %s to %s failed: %s\n", file, file_backup, 
		strerror( errno ) );
	goto cleanup;
    }

cleanup:
    if( file ) sqlite3_free( file );
    if( command ) sqlite3_free( command );
    if( file_backup ) sqlite3_free( file_backup );
    if( file_dump ) sqlite3_free( file_dump );

    if( pipe ) pclose( pipe );
    if( fpdump ) fclose( fpdump );

    return( rc );
#endif /* UNDER_CONSTRUCTION */
}
