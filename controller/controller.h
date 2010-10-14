void upk_db_reset_launchcallback( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
);
void upk_controller_bootstrap( sqlite3 *pdb );
