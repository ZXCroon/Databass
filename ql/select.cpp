#include "ql.h"
#include "../utils/utils.h"
#include <vector>


typedef std::pair<RID, RID> PRID;


RC QL_Manager::select(int nSelAttrs, const RelAttr selAttrs[],
                      const char* relation1, const char *relation2, JoinType joinType,
                      int nConditions, const Condition conditions[]) {

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
                (joinType == NO_JOIN || locateAttrcat(relation2, cat2, selAttrs[i]) == NULL)) {
            return QL_NO_SUCH_ATTRIBUTE;
        }
    }

    SelectStrategy strat;
    if (!decideStrategy(relation1, relation2, cat1, cat2, joinType, nConditions, conditions, strat)) {
        return QL_NO_SUCH_ATTRIBUTE;
    }


    RM_FileHandle *handle1, *handle2;
    rmm->openFile(getPath(smm->dbName, relation1), handle1);
    if (joinType != NO_JOIN) {
        rmm->openFile(getPath(smm->dbName, relation2), handle2);
    }
    IX_IndexHandle *ixHandle1, *ixHandle2;
    RM_FileScan fileScan;
    IX_IndexScan indexScan;
    RID rid;
    RM_Record rec, rec_;
    std::vector<RID> rids;
    std::vector<PRID> prids;

    if (joinType == NO_JOIN || strat.outer == 0) {
        std::vector<RID> rids1, rids2;
        if (strat.strat1.attrcat != NULL) {
            ixm->openIndex(getPath(smm->dbName, relation1), strat.strat1.attrcat->indexNo, ixHandle1);
            indexScan.openScan(*ixHandle1, strat.strat1.compOp,
                    padValue(strat.strat1.value.data, strat.strat1.attrcat->attrType, strat.strat1.attrcat->attrLength));
            while (true) {
                RC rc = indexScan.getNextEntry(rid);
                if (rc == IX_INDEXSCAN_EOF) {
                    break;
                }
                handle1->getRec(rid, rec);
                if (singleValidate(relation1, cat1, nConditions, conditions, rec)) {
                    rids1.push_back(rid);
                }
            }
            indexScan.closeScan();
            ixm->closeIndex(*ixHandle1);
        } else {
            fileScan.openScan(*handle1, 0, 0, 0, NO_OP, NULL);
            while (true) {
                RC rc = fileScan.getNextRec(rec);
                if (rc == RM_FILESCAN_NONEXT) {
                    break;
                }
                if (singleValidate(relation1, cat1, nConditions, conditions, rec)) {
                    rids1.push_back(rec.getRid());
                }
            }
            fileScan.closeScan();
        }
        rids = rids1;

        if (joinType != NO_JOIN) {
            if (strat.strat2.attrcat != NULL) {
                ixm->openIndex(getPath(smm->dbName, relation2), strat.strat2.attrcat->indexNo, ixHandle2);
                indexScan.openScan(*ixHandle2, strat.strat2.compOp,
                        padValue(strat.strat2.value.data, strat.strat2.attrcat->attrType, strat.strat2.attrcat->attrLength));
                while (true) {
                    RC rc = indexScan.getNextEntry(rid);
                    if (rc == IX_INDEXSCAN_EOF) {
                        break;
                    }
                    handle2->getRec(rid, rec);
                    if (singleValidate(relation2, cat2, nConditions, conditions, rec)) {
                        rids2.push_back(rid);
                    }
                }
                indexScan.closeScan();
                ixm->closeIndex(*ixHandle2);
            } else {
                fileScan.openScan(*handle2, 0, 0, 0, NO_OP, NULL);
                while (true) {
                    RC rc = fileScan.getNextRec(rec);
                    if (rc == RM_FILESCAN_NONEXT) {
                        break;
                    }
                    if (singleValidate(relation2, cat2, nConditions, conditions, rec)) {
                        rids2.push_back(rec.getRid());
                    }
                }
                fileScan.closeScan();
            }

            bool *visited1 = new bool[rids1.size()];
            bool *visited2 = new bool[rids2.size()];
            memset(visited1, false, rids1.size());
            memset(visited2, false, rids2.size());
            RM_Record rec1, rec2;
            for (int i = 0; i < rids1.size(); ++i) {
                for (int j = 0; j < rids2.size(); ++j) {
                    handle1->getRec(rids1[i], rec1);
                    handle2->getRec(rids2[j], rec2);
                    if (pairValidate(relation1, relation2, cat1, cat2, nConditions, conditions, rec1, rec2)) {
                        prids.push_back(std::make_pair(rids1[i], rids2[j]));
                        visited1[i] = visited2[j] = true;
                    }
                }
            }

            if (joinType == LEFT_JOIN || joinType == FULL_JOIN) {
                for (int i = 0; i < rids1.size(); ++i) {
                    if (!visited1[i]) {
                        prids.push_back(std::make_pair(rids1[i], RID(-1, -1)));
                    }
                }
            }
            if (joinType == RIGHT_JOIN || joinType == FULL_JOIN) {
                for (int j = 0; j < rids2.size(); ++j) {
                    if (!visited2[j]) {
                        prids.push_back(std::make_pair(RID(-1, -1), rids2[j]));
                    }
                }
            }
            delete[] visited1;
            delete[] visited2;
        } 
    }

    // nested-loop case

    const RelAttr *selAttrs_ = selAttrs;
    RelAttr *selAttrs__;
    int nSelAttrs_ = nSelAttrs;
    // SELECT *
    if (nSelAttrs < 0) {
        if (joinType == NO_JOIN) {
            nSelAttrs_ = cat1.relcat.attrCount;
            selAttrs__ = new RelAttr[nSelAttrs_];
            for (int i = 0; i < nSelAttrs_; ++i) {
                selAttrs__[i].relName = NULL;
                selAttrs__[i].attrName = cat1.attrcats[i].attrName;
            }
        } else {
            nSelAttrs_ = cat1.relcat.attrCount + cat2.relcat.attrCount;
            selAttrs__ = new RelAttr[nSelAttrs_];
            for (int i = 0; i < cat1.relcat.attrCount; ++i) {
                selAttrs__[i].relName = relation1;
                selAttrs__[i].attrName = cat1.attrcats[i].attrName;
            }
            for (int i = 0; i < cat2.relcat.attrCount; ++i) {
                selAttrs__[i + cat1.relcat.attrCount].relName = relation2;
                selAttrs__[i + cat1.relcat.attrCount].attrName = cat2.attrcats[i].attrName;
            }
        }
        selAttrs_ = selAttrs__;
    }
    // output
    std::cout << "|";
    for (int i = 0; i < nSelAttrs_; ++i) {
        std::cout << " ";
        print(selAttrs_[i].attrName, VARSTRING, MAXNAME + 1);
        std::cout << " |";
    }
    std::cout << std::endl;
    if (joinType == NO_JOIN) {
        for (int i = 0; i < rids.size(); ++i) {
            std::cout << "|";
            handle1->getRec(rids[i], rec);
            for (int j = 0; j < nSelAttrs_; ++j) {
                const AttrcatLayout *ac = locateAttrcat(relation1, cat1, selAttrs_[j]);
                std::cout << " ";
                print(rec.getData() + ac->offset, ac->attrType, ac->attrLength);
                std::cout << " |";
            }
            std::cout << std::endl;
        }
    } else {
        for (int i = 0; i < prids.size(); ++i) {
            std::cout << "|";
            if (prids[i].first != RID(-1, -1)) {
                handle1->getRec(prids[i].first, rec);
            }
            if (prids[i].second != RID(-1, -1)) {
                handle2->getRec(prids[i].second, rec_);
            }
            for (int j = 0; j < nSelAttrs_; ++j) {
                std::cout << " ";
                const AttrcatLayout *ac = locateAttrcat(relation1, cat1, selAttrs_[j]);
                const AttrcatLayout *ac_ = locateAttrcat(relation2, cat2, selAttrs_[j]);
                if (ac != NULL && prids[i].first != RID(-1, -1)) {
                    print(rec.getData() + ac->offset, ac->attrType, ac->attrLength);
                } else if (ac_ != NULL && prids[i].second != RID(-1, -1)) {
                    print(rec_.getData() + ac_->offset, ac_->attrType, ac_->attrLength);
                } else {
                    std::cout << "NULL";
                }
                std::cout << " |";
            }
            std::cout << std::endl;
        }
    }

    rmm->closeFile(*handle1);
    if (joinType != NO_JOIN) {
        rmm->closeFile(*handle2);
    }
    if (nSelAttrs < 0) {
        delete[] selAttrs_;
    }
    return 0;
}


bool QL_Manager::singleValidate(const char *relation, const Catalog &cat, int nConditions, const Condition *conditions, const RM_Record &rec) {
    for (int i = 0; i < nConditions; ++i) {
        Condition cond = conditions[i];
        const AttrcatLayout *ac = locateAttrcat(relation, cat, cond.lhsAttr);
        if (!ac) {
            continue;
        }
        if (cond.bRhsIsAttr) {
            const AttrcatLayout *ac_ = locateAttrcat(relation, cat, cond.rhsAttr);
            if (ac_ != NULL && !validate(rec.getData() + ac->offset, ac->attrType, ac->attrLength, cond.op, rec.getData() + ac_->offset)) {
                return false;
            }
        }
        else if (!validate(rec.getData() + ac->offset, ac->attrType, ac->attrLength, cond.op,
                    padValue(cond.rhsValue.data, ac->attrType, ac->attrLength))) {
            return false;
        }
    }
    return true;
}


bool QL_Manager::pairValidate(const char *relation1, const char *relation2,
                              const Catalog &cat1, const Catalog &cat2,
                              int nConditions, const Condition *conditions,
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
        const AttrcatLayout *acl = locateAttrcat(relation1, cat1, cond.lhsAttr);
        const AttrcatLayout *acr = locateAttrcat(relation2, cat2, cond.rhsAttr);
        if (acl != NULL && acr != NULL) {
            if (!validate(rec1.getData() + acl->offset, acl->attrType, acl->attrLength, cond.op, rec2.getData() + acr->offset)) {
                return false;
            }
        }
    }

    // hidden equalities
    /*
    for (int i = 0; i < cat1.relcat.attrCount; ++i) {
        for (int j = 0; j < cat2.relcat.attrCount; ++j) {
            if (strcmp(cat1.attrcats[i].attrName, cat2.attrcats[j].attrName) != 0) {
                continue;
            }
            RelAttr ra = {NULL, cat1.attrcats[i].attrName};
            const AttrcatLayout *acl = locateAttrcat(relation1, cat1, ra);
            const AttrcatLayout *acr = locateAttrcat(relation2, cat2, ra);
            if (acl != NULL && acr != NULL) {
                if (!validate(rec1.getData() + acl->offset, acl->attrType, acl->attrLength, EQ_OP, rec2.getData() + acr->offset)) {
                    return false;
                }
            }
        }
    }
    */
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
            Condition cond = conditions[i];
            if (cond.bRhsIsAttr || cond.op == NE_OP || cond.op == NO_OP) {
                continue;
            }
            const AttrcatLayout *ac = locateAttrcat(relation1, cat1, cond.lhsAttr);
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
        Condition cond = conditions[i];
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
                const AttrcatLayout *ac = locateAttrcat(relation1, cat1, cond.lhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat1.attrcat = ac;
                    strat.strat1.compOp = cond.op;
                    if (cond.bRhsIsAttr) {
                        strat.strat1.value.data = NULL;
                        const AttrcatLayout *ac_ = locateAttrcat(relation2, cat2, cond.rhsAttr);
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
                const AttrcatLayout *ac = locateAttrcat(relation1, cat1, cond.rhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat1.attrcat = ac;
                    strat.strat1.compOp = cond.op;
                    strat.strat1.value.data = NULL;
                    const AttrcatLayout *ac_ = locateAttrcat(relation2, cat2, cond.rhsAttr);
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
                const AttrcatLayout *ac = locateAttrcat(relation2, cat2, cond.lhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat2.attrcat = ac;
                    strat.strat2.compOp = cond.op;
                    if (cond.bRhsIsAttr) {
                        strat.strat2.value.data = NULL;
                        const AttrcatLayout *ac_ = locateAttrcat(relation1, cat1, cond.rhsAttr);
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
                const AttrcatLayout *ac = locateAttrcat(relation2, cat2, cond.rhsAttr);
                if (ac->indexNo != -1) {
                    strat.strat2.attrcat = ac;
                    strat.strat2.compOp = cond.op;
                    strat.strat2.value.data = NULL;
                    const AttrcatLayout *ac_ = locateAttrcat(relation1, cat1, cond.rhsAttr);
                    if (ac_ == NULL) {
                        return false;
                    }
                    strat.strat2.auxAttrcat = ac_;
                }
            }
        }
    }

    // decide strategy according to hidden equalities of join attrs
    /*
    if (strat.strat1.attrcat == NULL || strat.strat2.attrcat == NULL) {
        for (int i = 0; i < cat1.relcat.attrCount; ++i) {
            RelAttr ra = {NULL, cat1.attrcats[i].attrName};
            const AttrcatLayout *ac = locateAttrcat(relation2, cat2, ra);
            if (ac == NULL) {
                continue;
            }
            if (strat.strat1.attrcat == NULL && cat1.attrcats[i].indexNo != -1) {
                strat.strat1.attrcat = &(cat1.attrcats[i]);
                strat.strat1.compOp = EQ_OP;
                strat.strat1.value.data = NULL;
                strat.strat1.auxAttrcat = ac;
            }
            if (strat.strat2.attrcat == NULL && ac->indexNo != -1) {
                strat.strat2.attrcat = ac;
                strat.strat2.compOp = EQ_OP;
                strat.strat2.value.data = NULL;
                strat.strat2.auxAttrcat = &(cat1.attrcats[i]);
            }
        }
    }
    */


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
        } else if (strat.strat1.attrcat != NULL && strat.strat1.auxAttrcat != NULL) {
            strat.outer = 2;
        } else {
            strat.outer = 0;
        }
    } else {
        // if cross-relation attributes cannot be leveraged, may as well post-join
        strat.outer = 0;
    }
    
    if (strat.outer != 2 && strat.strat1.auxAttrcat != NULL) {
        strat.strat1.attrcat = strat.strat1.auxAttrcat = NULL;
    }
    if (strat.outer != 1 && strat.strat2.auxAttrcat != NULL) {
        strat.strat2.attrcat = strat.strat2.auxAttrcat = NULL;
    }

    return true;
}
