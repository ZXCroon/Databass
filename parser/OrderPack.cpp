#include "OrderPack.h"

void OrderPack::process() {
    switch (type) {

    case SHOW_DATABASES: {
        smm.showDbs();
        break;
    }

    case CREATE_DATABASE: {
        printf("DEBUG: CREATE %s\n", dbname);
        smm.createDb(dbname);
        break;
    }

    case DROP_DATABASE: {
        printf("DEBUG: DROP %s\n", dbname);
        smm.dropDb(dbname);
        break;
    }

    case USE_DATABASE: {
        smm.openDb(dbname);
        break;
    }

    case SHOW_TABLES: {
        // todo
        break;
    }
    
    case EXIT: {
        //todo
        printf("EXIT!\n");
        exit(0);
    }
    }
}