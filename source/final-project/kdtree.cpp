#include "kdtree.h"


Kdtree::Node::Node(Kdpack _item) : left(nullptr), right(nullptr)
{
    key[0] = _item.key[0];
    key[1] = _item.key[1];
    value = _item.value;
}

void Kdtree::clear(Node *root)
{
    if (root == nullptr)
        return;
    clear(root->left);
    clear(root->right);
    delete root;
}

Kdtree::Kdtree()
{
    root = nullptr;
}

Kdtree::~Kdtree()
{
    clear(root);
}

void Kdtree::insert(Node *&root, Kdpack item, int dim)
{
    if (root == nullptr)
    {
        root = new Node(item);
        return;
    }
    if (item.key[0] == root->key[0] && item.key[1] == root->key[1])
    {
        return;
    }
    if (item.key[dim] < root->key[dim])
        insert(root->left, item, !dim);
    else
        insert(root->right, item, !dim);
}

void Kdtree::insert(Kdpack item)
{
    insert(root, item, 0);
}

QVector<int> Kdtree::rangeSearch(Node *root, int dim, double *lower_bound, double *upper_bound)
{
    QVector<int> ret;
    if (root == nullptr)
        return ret;

    if ((lower_bound[0] <= root->key[0] && root->key[0] <= upper_bound[0]) && (lower_bound[1] <= root->key[1] && root->key[1] <= upper_bound[1]))
    {
        ret.push_back(root->value);
    }
    if (root->left != nullptr)
    {
        QVector<int> left;
        left = rangeSearch(root->left, !dim, lower_bound, upper_bound);
        for (int i = 0; i < left.size(); i++)
        {
            ret.push_back(left[i]);
        }
    }
    if (root->right != nullptr)
    {
        QVector<int> right;
        right = rangeSearch(root->right, !dim, lower_bound, upper_bound);
        for (int i = 0; i < right.size(); i++)
        {
            ret.push_back(right[i]);
        }
    }
    return ret;
}

QVector<int> Kdtree::rangeSearch(double lowX, double highX, double lowY, double highY)
{
    double lowerBound[2] = {lowX, lowY};
    double higherBound[2] = {highX, highY};
    return rangeSearch(root, 0, lowerBound, higherBound);
}
