#ifndef KDTREE_H
#define KDTREE_H

#include <QVector>

struct Kdpack
{
    double key[2]; //key[0] for latitude, key[1] for longitude
    int value;
    Kdpack(double _latitude, double _longitude, int _value)
    {
        key[0] = _latitude;
        key[1] = _longitude;
        value = _value;
    }
};

class Kdtree
{
public:
    struct Node
    {
        Node *left, *right;
        double key[2];
        int value;
        Node(Kdpack _item);
    };
    Node *root;

    QVector<int> rangeSearch(Node *root, int dim, double *lower_bound, double *upper_bound);
    void insert(Node *&root, Kdpack item, int dim);
    void clear(Node *rt);

public:
    Kdtree();
    ~Kdtree();
    void insert(Kdpack item);
    QVector<int> rangeSearch(double lowX, double highX, double lowY, double highY);
};

#endif // KDTREE_H
