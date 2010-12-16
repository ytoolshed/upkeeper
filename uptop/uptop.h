
void uptop_print_callback( 
    void *ignored,                      
    upk_srvc_t  srvc,                                    
    const char *status_desired,
    const char *status_actual
);

void uptop_services_print(
    sqlite3 *pdb
);
