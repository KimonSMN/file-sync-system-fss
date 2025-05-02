Για το compilation της ασκησης τρεξετε `make`.  
Για να τρεξετε την ασκηση: ./build/fss_manager -l <path_to_manager_log> -c <path_to_config> -n <numberOfWorkers>

Στην άσκησή μου, χρησιμοποιώ ως sync_info_mem_store ενα global hash table επειδη την διαχειρίζονται πολλά αρχεία.
Επίσης χρησιμοποιώ ενα global queue για την αποθήκευση εργασιών που περιμένουν να τρέξουν όταν δεν υπάρχουν διαθέσιμοι workers.
Εχω προσθεσει ενα signal mask για τα SIGCHLD που το blockαρει οσο διαβάζονται τα <source, target> απο το config_file. Χώρις το block 
υπάρχει περίπτωση να μην διαβαστεί ολο το config_file.

Δεν κατάφερα να υλοποιήσω το exec_report,  ούτε να στέλνω μηνύματα τυπου:[TIMESTAMP] [SOURCE_DIR] [TARGET_DIR] [WORKER_PID] [OPERATION] [RESULT] [DETAILS]
διότι ο παραλληλισμός της άσκησης σταματούσε να λειτουργεί σωστά εφόσον οι workers περιμένανε στο read (spawn_worker commented κώδικας).

Απο το fss_script δεν εχω υλοποιήσει listAll, listMonitored.