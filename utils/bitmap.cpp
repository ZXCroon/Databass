#include "utils.h"
#include <string>
#include <iostream>
#include <cstring>


template <typename T>
int findRightMostInInteger(T x, int b) {
    int scale = sizeof(T) * 8;
    int res = 0;
    while (scale >= 8) {
        T left = x >> (scale / 2);
        T right = x & ((1LL << (scale / 2)) - 1);
        if (b == 0 && right != (1LL << (scale  / 2)) - 1 || b == 1 && right != 0) {
            x = right;
        } else if (b == 0 && left != (1LL << (scale  / 2)) - 1 || b == 1 && left != 0) {
            x = left;
            res += scale / 2;
        } else return -1;
        scale /= 2;
    }
    if ((x & 1) == b) {
        return res;
    }
    if (((x >> 1) & 1) == b) {
        return res + 1;
    }
    if (((x >> 2) & 1) == b) {
        return res + 2;
    }
    if (((x >> 3) & 1) == b) {
        return res + 3;
    }
    return -1;
}

void initBitMap(struct BitMap &bm) {
    memset(bm.leaf, 0, sizeof(bm.leaf));
    bm.rootForEmpty = -1;
    bm.rootForFull = 0;
}

void setBit(struct BitMap &bm, int index, int b) {
    int k = index / 64, t = index % 64;
    if (b == 0) {
        bm.leaf[k] &= ~(1LL << t);
        bm.rootForFull &= US(~(1 << k));
        if (bm.leaf[k] == 0) {
            bm.rootForEmpty |= US(1 << k);
        }
    } else {
        bm.leaf[k] |= (1LL << t);
        bm.rootForEmpty &= US(~(1 << k));
        if (bm.leaf[k] == -1) {
            bm.rootForFull |= US(1 << k);
        }
    }
}

int queryBit(struct BitMap bm, int index) {
    return (bm.leaf[index / 64] >> (index % 64)) & 1;
}

int findRightMost(struct BitMap bm, int b) {
    int k;
    if (b == 0) {
        k = findRightMostInInteger(bm.rootForFull, 0);
    } else {
        k = findRightMostInInteger(bm.rootForEmpty, 0);
    }
    if (k == -1) {
        return -1;
    }
    return k * 64 + findRightMostInInteger(bm.leaf[k], b);
}

void print(const struct BitMap &bm) {
    for (int i = 0; i < 16; ++i) {
        std::cout << bm.leaf[i] << " ";
    }
    std::cout << std::endl;
}
