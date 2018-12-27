#include "OrderPack.h"

void OrderPack::process() {
    switch (type) {

    case SHOW_DATABASES: {
        smm.showDbs();
        break;
    }

    case EXIT: {
        exit(0);
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
        smm.showTables();
        break;
    }

    case CREATE_TABLE: {
        // todo
        break;
    }

    case DROP_TABLE: {
        smm.dropTable(tbname);
        break;
    }

    case DESC_TABLE: {
        smm.showTable(tbname);
        break;
    }

    case INSERT_VALUES: {
        // todo
        break;
    }

    case DELETE_VALUES: {
        // todo
        break;
    }

    case UPDATE_VALUES: {
        // todo
        break;
    }

    case SELECT_VALUES: {
        // todo
        break;
    }

    case CREATE_INDEX: {
        smm.createIndex(tbname, colname);
        break;
    }

    case DROP_INDEX: {
        smm.dropIndex(tbname, colname);
        break;
    }
}