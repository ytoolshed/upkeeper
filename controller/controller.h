void upk_db_reset_launchcallback( 
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
);
void upk_controller_bootstrap( sqlite3 *pdb );
void upk_controller_status_fixer( sqlite3 *pdb, const char *db );
  
