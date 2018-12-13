#include "ql.h"
#include <vector>


RC QL_Manager::select(int nSelAttrs, const RelAttr selAttrs[],
                        const char *relation1, const char *relation2, const JoinType &joinType,
                        int nConditions,
                        const Condition conditions[]) {

    Catalog cat1, cat2;
    if (!getCatalog(relation1, cat1)) {
        return QL_NO_SUCH_RELATION;
    }
    if (joinType != NO_JOIN && !getCatalog(relation2, cat2)) {
        return QL_NO_SUCH_RELATION;
    }
 
    // check selAttrs
    for (int i = 0; i < nSelAttrs; ++i) {
        if (locateAttrcat(relation1, cat1, selAttrs[i]) == NULL &&
                (joinType == NO_JOIN || locateAttrcat(relation2, cat2, selAttrs[i]) == NULL) {
            return QL_NO_SUCH_ATTRIBUTE;
        }
    }

    SelectStrategy strat;
    if (!decideStrategy(relation1, relation2, cat1, cat2, joinType, nConditions, conditions, strat)) {
        return QL_NO_SUCH_ATTRIBUTE;
    }


    RM_FileHandle handle1, handle2;
    rmm->openFile(getPath(dbName, relation1), handle1);
    if (joinType != NO_JOIN) {
        rmm->openFile(getPath(dbName, relation2), handle2);
    }
    IX_IndexHandle ixHandle1, ixHandle2;
    RM_FileScan fileScan;
    IX_IndexScan indexScan;
    RID rid;
    RM_Record rec, rec_;

    if (joinType == NO_JOIN || strat == 0) {
        std::vector<RID> rids1, rids2;
        if (strat.strat1.attrcat != NULL) {
            ixm->openIndex(getPath(dbName, relation1), strat.strat1.attrcat->indexNo, ixHandle1);
            indexScan.openScan(ixHandle1, strat.strat1.compOp, padValue(strat.strat1.value));
            while (true) {
                RC rc = indexScan.getNextEntry(rid);
                if (rc == IX_INDEXSCAN_NONEXT) {
                    break;
                }
                handle1.getRec(rid, rec);
                if (singleValidate(relation1, cat1, nConditions, conditions, rec)) {
                    rids1.append(rid);
                }
            }
            indexScan.closeScan();
            ixm->closeIndex(ixHandle1);
        } else {
            fileScan.openScan(handle1, 0, 0, 0, NO_OP, NULL);
            while (true) {
                RC rc = fileScan.getNextRec(rec);
                if (rc == RM_FILESCAN_NONEXT) {
                    break;
                }
                if (singleValidate(relation1, cat1, nConditions, conditions, rec)) {
                    rids1.append(rid);
                }
            }
            fileScan.closeScan();
        }

        if (joinType != NO_JOIN) {
            if (strat.strat2.attrcat != NULL) {
                ixm->openIndex(getPath(dbName, relation2), strat.strat2.attrcat->indexNo, ixHandle2);
                indexScan.openScan(ixHandle2, strat.strat2.compOp, padValue(strat.strat2.value));
                while (true) {
                    RC rc = indexScan.getNextEntry(rid);
                    if (rc == IX_INDEXSCAN_NONEXT) {
                        break;
                    }
                    handle2.getRec(rid, rec);
                    if (singleValidate(relation2, cat2, nConditions, conditions, rec)) {
                        rids2.append(rid);
                    }
                }
                indexScan.closeScan();
                ixm->closeIndex(ixHandle2);
            } else {
                fileScan.openScan(handle2, 0, 0, 0, NO_OP, NULL);
                while (true) {
                    RC rc = fileScan.getNextRec(rec);
                    if (rc == RM_FILESCAN_NONEXT) {
                        break;
                    }
                    if (singleValidate(relation2, cat2, nConditions, conditions, rec)) {
                        rids2.append(rid);
                    }
                }
                fileScan.closeScan();
            }

            bool *visited1 = new bool[rids1.size()]
            bool *visited2 = new bool[rids2.size()]
            memset(visited1, false, rids1.size());
            memset(visited2, false, rids2.size());
            for (int i = 0; i < rids1.size(); ++i) {
                for (int j = 0; j < rids2.size(); ++j) {
                    handle1.getRec(rids1[i], rec1);
                    handle2.getRec(rids2[i], rec2);
                    if (pairValidate(relation1, relation2, cat1, cat2, nConditions, conditions, rec1, rec2)) {
                        // TODO
                        visited1[i] = visited2[j] = True;
                    }
                }
            }

            if (joinType == LEFT_JOIN || joinType == FULL_JOIN) {
                for (int i = 0; i < rids1.size(); ++i) {
                    if (!visited1[i]) {
                        // TODO
                    }
                }
            }
            if (joinType == RIGHT_JOIN || joinType == FULL_JOIN) {
                for (int j = 0; j < rids2.size(); ++j) {
                    if (!visited2[j]) {
                        // TODO
                    }
                }
            }
            delete[] visited1;
            delete[] visited2;
        }
    }

    rmm->closeFile(handle1);
    if (joinType != NO_JOIN) {
        rmm->closeFile(handle2);
    }
    return 0;
}


bool QL_Manager::singleValidate(const char *relation, const Catalog &cat, int nConditions, const Condition conditions, const RM_Record &rec) {
    for (int i = 0; i < nConditions; ++i) {
        Condition cond = conditions[i];
        AttrcatLayout *ac = locateAttrcat(relation, cat, cond.lhsAttr);
        if (!ac) {
            continue;
        }
        if (cond.bRhsIsAttr) {
            AttrcatLayout *ac_ = locateAttrcat(relation, cat, cond.rhsAttr);
            if (ac_ != NULL && !validate(rec.getData() + ac->offset, ac->attrType, ac->attrLength, cond.op, rec.getData() + ac_->offset)) {
                return false;
            }
        }
        else if (!validate(rec.getData() + ac->offset, ac->attrType, ac->attrLength, cond.op, padValue(cond.rhsValue.value))) {
            return false;
        }
    }
    return true;
}


bool QL_Manager::pairValidate(const char *relation1, const char *relation2,
                              const Catalog &cat1, const Catalog &cat2,
                              int nConditions, const Condition conditions,
                              const RM_Record &rec1, const RM_Record &rec2) {
    if (!singleValidate(relation1, cat1, nConditions, conditions, rec1)) {
        return false;
    }
    if (!singleValidate(relation2, cat2, nConditions, conditions, rec2)) {
        return false;
    }

    for (int i = 0; i < nConditions; ++i) {
        Condition cond = conditions[i];
        if (!cond.bRhsIsAttr) {
            continue;
        }
        AttrcatLayout *acl = locateAttrcat(relation1, cat1, cond.lhsAttr);
        AttrcatLayout *acr = locateAttrcat(relation2, cat2, cond.rhsAttr);
        if (acl != NULL && acr != NULL) {
            if (!validate(rec1.getData() + acl->offset, acl->attrType, acl->attrLength, cond.op, rec2.getData() + acr->offset)) {
                return false;
            }
        }
    }
    return true;
}


bool QL_Manager::decideStrategy(const char *relation1, const char *relation2,
                                          const Catalog &cat1, const Catalog &cat2, const JoinType &joinType,
                                          int nConditions, const Condition conditions[], SelectStrategy &strat) {
    strat.strat1.attrcat = strat.strat2.attrcat = NULL;
    strat.strat1.auxAttrcat = strat.strat2.auxAttrcat = NULL;
    strat.outer = -1;

    if (joinType == NO_JOIN) {
        for (int i = 0; i < nConditions; ++i) {
            cond = conditions[i];
            if (cond.bRhsIsAttr || cond.op == NE_OP || cond.op == NO_OP) {
                continue;
            }
            AttrcatLayout *ac = locateAttrcat(relation1, cat1, cond.lhsAttr);
            if (ac == NULL) {
                return false;
            }
            if (ac->indexNo != -1) {
                strat.strat1.attrcat = ac;
                strat.strat1.value = cond.rhsValue;
                return true;
            }
        }
        return true;
    }

    for (int i = 0; i < nConditions; ++i) {
        cond = conditions[i];
        if (cond.op == NE_OP || cond.op == NO_OP) {
            continue;
        }

        int lhsBelong = -1, rhsBelong = -1;
        if (locateAttrcat(relation1, cat1, cond.lhsAttr) != NULL) {
            lhsBelong = 1;
        }
        if (locateAttrcat(relation2, cat2, cond.lhsAttr) != NULL) {
            lhsBelong = (lhsBelong == -1 ? 2 : 0);
        }
        if (lhsBelong == -1) {
            return false;
        }
        if (cond.bRhsIsAttr) {
            if (locateAttrcat(relation1, cat1, cond.rhsAttr) != NULL) {
                rhsBelong = 1;
            }
            if (locateAttrcat(relation2, cat2, cond.rhsAttr) != NULL) {
                rhsBelong = (rhsBelong == -1 ? 2 : 0);
            }
            if (rhsBelong == -1) {
                return false;
            }
        }
        if (lhsBelong == rhsBelong && lhsBelong != 0) {
            continue;
        }

        // decide strategy for relation1
        if (strat.strat1.attrcat == NULL || strat.strat1.auxAttrcat != NULL) {
            if (lhsBelong == 1 || lhsBelong == 0) {
                AttrcatLayout *ac = locateAttrcat(relation1, cat1, cond.lhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat1.attrcat = ac;
                    strat.strat1.compOp = cond.op;
                    if (cond.bRhsIsAttr) {
                        strat.strat1.value = NULL;
                        AttrcatLayout *ac_ = locateAttrcat(relation2, cat2, cond.rhsAttr);
                        if (ac_ == NULL) {
                            return false;
                        }
                        strat.strat1.auxAttrcat = ac;
                    } else {
                        strat.strat1.value = cond.rhsValue;
                        strat.strat1.auxAttrcat = NULL;
                    }
                }
            } else if (rhsBelong == 1 || rhsBelong == 0) {
                AttrcatLayout *ac = locateAttrcat(relation1, cat1, cond.rhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat1.attrcat = ac->attrcat;
                    strat.strat1.compOp = cond.op;
                    strat.strat1.value = NULL;
                    AttrcatLayout *ac_ = locateAttrcat(relation2, cat2, cond.rhsAttr);
                    if (ac_ == NULL) {
                        return false;
                    }
                    strat.strat1.auxAttrcat = ac_;
                }
            }
        }

        // decide strategy for relation2
        if (strat.strat2.attrcat == NULL || strat.strat2.auxAttrcat != NULL) {
            if (lhsBelong == 2 || lhsBelong == 0) {
                AttrcatLayout *ac = locateAttrcat(relation2, cat2, cond.lhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat2.attrcat = ac;
                    strat.strat2.compOp = cond.op;
                    if (cond.bRhsIsAttr) {
                        strat.strat2.value = NULL;
                        AttrcatLayout *ac_ = locateAttrcat(relation1, cat1, cond.rhsAttr);
                        if (ac_ == NULL) {
                            return false;
                        }
                        strat.strat2.auxAttrcat = ac;
                    } else {
                        strat.strat2.value = cond.rhsValue;
                        strat.strat2.auxAttrcat = NULL;
                    }
                }
            } else if (rhsBelong == 2 || rhsBelong == 0) {
                AttrcatLayout *ac = locateAttrcat(relation2, cat2, cond.rhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat2.attrcat = ac->attrcat;
                    strat.strat2.compOp = cond.op;
                    strat.strat2.value = NULL;
                    AttrcatLayout *ac_ = locateAttrcat(relation1, cat1, cond.rhsAttr);
                    if (ac_ == NULL) {
                        return false;
                    }
                    strat.strat2.auxAttrcat = ac_;
                }
            }
        }
    }

    // decide strategy according to hidden equalities of join attrs
    if (strat.stra1.attrcat == NULL || strat.strat2.attrcat == NULL) {
        for (int i = 0; i < cat1.relcat.attrCount; ++i) {
            RelAttr ra = {NULL, cat.attrcats[i]}
            AttrcatLayout *ac = locateAttrcat(relation2, cat2, ra);
            if (ac == NULL) {
                continue;
            }
            if (strat.strat1.attrcat == NULL && cat1.relcat.attrcats[i].indexNo != -1) {
                strat.strat1.attrcat = &(cat1.attrcats[i]);
                strat.strat1.compOp = EQ_OP;
                strat.strat1.value = NULL;
                strat.strat1.auxAttrcat = ac;
            }
            if (strat.strat2.attrcat == NULL && ac->indexNo != -1) {
                strat.strat2.attrcat = ac;
                strat.strat2.compOp = EQ_OP;
                strat.strat2.value = NULL;
                strat.strat2.auxAttrcat = &(cat1.attrcats[i]);
            }
        }
    }


    /*
     * finalize strategy
     */
    if (joinType == LEFT_JOIN && strat.strat2.attrcat != NULL && strat.strat2.auxAttrcat != NULL) {
        strat.outer = 1;
    } else if (joinType == RIGHT_JOIN && strat.strat1.attrcat != NULL && strat.strat1.auxAttrcat != NULL) {
        strat.outer = 2;
    } else if (joinType == INNER_JOIN) {
        if (strat.strat2.attrcat != NULL && strat.strat2.auxAttrcat != NULL) {
            strat.outer = 1;
        }
        if (strat.strat1.attrcat != NULL && strat.strat1.auxAttrcat != NULL) {
            strat.outer = 2;
        }
    } else {
        // if cross-relation attributes cannot be leveraged, may as well post-join
        strat = 0;
    }
    
    if (strat.outer != 2 && strat.strat1.auxAttrcat != NULL) {
        strat.strat1.attrcat = strat.strat1.auxAttrcat = NULL;
    }
    if (strat.outer != 1 && strat.strat2.auxAttrcat != NULL) {
        strat.strat2.attrcat = strat.strat2.auxAttrcat = NULL;
    }

    return true;
}
