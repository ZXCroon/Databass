#include "OrderPack.h"

void OrderPack::process() {
    switch (type) {

    case SHOW_DATABASES: {
        // todo
        break;
    }

    case CREATE_DATABASE: {
        smm.createDb(dbname);
        break;
    }

    case DROP_DATABASE: {
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
    
    }
}