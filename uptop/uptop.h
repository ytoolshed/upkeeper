
void uptop_print_callback( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
);

void uptop_services_print(
    sqlite3 *pdb
);
