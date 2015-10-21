#include "offerwhitelisttablemodel.h"

#include "guiutil.h"
#include "walletmodel.h"

#include "wallet.h"
#include "base58.h"

#include <QFont>
using namespace std;
using namespace json_spirit;

extern const CRPCTable tableRPC;
struct OfferWhitelistTableEntry
{

	QString cert;
    QString title;
	QString mine;
    QString address;
	QString expires;
	QString discount;	

    OfferWhitelistTableEntry() {}
    OfferWhitelistTableEntry(const QString &cert, const QString &title, const QString &mine,const QString &address, const QString &expires,const QString &discount):
        cert(cert), title(title), mine(mine),address(address), expires(expires),discount(discount) {}
};

struct OfferWhitelistTableEntryLessThan
{
    bool operator()(const OfferWhitelistTableEntry &a, const OfferWhitelistTableEntry &b) const
    {
        return a.cert < b.cert;
    }
    bool operator()(const OfferWhitelistTableEntry &a, const QString &b) const
    {
        return a.cert < b;
    }
    bool operator()(const QString &a, const OfferWhitelistTableEntry &b) const
    {
        return a < b.cert;
    }
};

#define NAMEMAPTYPE map<vector<unsigned char>, uint256>

// Private implementation
class OfferWhitelistTablePriv
{
public:
    QList<OfferWhitelistTableEntry> cachedEntryTable;
    OfferWhitelistTableModel *parent;

    OfferWhitelistTablePriv(OfferWhitelistTableModel *parent):
        parent(parent) {}


    void updateEntry(const QString &cert, const QString &title, const QString &mine,const QString &address, const QString &expires,const QString &discount, int status)
    {
		if(!parent)
		{
			return;
		}
        // Find offer / value in model
        QList<OfferWhitelistTableEntry>::iterator lower = qLowerBound(
            cachedEntryTable.begin(), cachedEntryTable.end(), cert, OfferWhitelistTableEntryLessThan());
        QList<OfferWhitelistTableEntry>::iterator upper = qUpperBound(
            cachedEntryTable.begin(), cachedEntryTable.end(), cert, OfferWhitelistTableEntryLessThan());
        int lowerIndex = (lower - cachedEntryTable.begin());
        int upperIndex = (upper - cachedEntryTable.begin());
        bool inModel = (lower != upper);

        switch(status)
        {
        case CT_NEW:
            if(inModel)
            {
                OutputDebugStringF("Warning: OfferWhitelistTablePriv::updateEntry: Got CT_NOW, but entry is already in model\n");
                break;
            }
            parent->beginInsertRows(QModelIndex(), lowerIndex, lowerIndex);
            cachedEntryTable.insert(lowerIndex, OfferWhitelistTableEntry(cert, title, mine, address, expires, discount));
            parent->endInsertRows();
            break;
        case CT_UPDATED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: OfferWhitelistTablePriv::updateEntry: Got CT_UPDATED, but entry is not in model\n");
                break;
            }
            lower->cert = cert;
			lower->title = title;
			lower->mine = mine;
			lower->address = address;
			lower->expires = expires;
			lower->discount = discount;
            parent->emitDataChanged(lowerIndex);
            break;
        case CT_DELETED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: OfferWhitelistTablePriv::updateEntry: Got CT_DELETED, but entry is not in model\n");
                break;
            }
            parent->beginRemoveRows(QModelIndex(), lowerIndex, upperIndex-1);
            cachedEntryTable.erase(lower, upper);
            parent->endRemoveRows();
            break;
        }
    }

    int size()
    {
        return cachedEntryTable.size();
    }

    OfferWhitelistTableEntry *index(int idx)
    {
        if(idx >= 0 && idx < cachedEntryTable.size())
        {
            return &cachedEntryTable[idx];
        }
        else
        {
            return 0;
        }
    }
};

OfferWhitelistTableModel::OfferWhitelistTableModel(WalletModel *parent) :
    QAbstractTableModel(parent)
{
    columns << tr("GUID") << tr("Title") << tr("Owner") << tr("Address") << tr("Expires In") << tr("Discount");
    priv = new OfferWhitelistTablePriv(this);

}

OfferWhitelistTableModel::~OfferWhitelistTableModel()
{
    delete priv;
}
int OfferWhitelistTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int OfferWhitelistTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

QVariant OfferWhitelistTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    OfferWhitelistTableEntry *rec = static_cast<OfferWhitelistTableEntry*>(index.internalPointer());

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Cert:
            return rec->cert;
        case Title:
            return rec->title;
        case Mine:
            return rec->mine;
        case Address:
            return rec->address;
        case Discount:
            return rec->discount;
        case Expires:
            return rec->expires;
        }
    }
    else if (role == Qt::FontRole)
    {
        QFont font;
        if(index.column() == Address)
        {
            font = GUIUtil::bitcoinAddressFont();
        }
        return font;
    }
    else if (role == CertRole)
    {
        return rec->cert;
    }
    return QVariant();
}

bool OfferWhitelistTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;
    OfferWhitelistTableEntry *rec = static_cast<OfferWhitelistTableEntry*>(index.internalPointer());

    editStatus = OK;

    if(role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Title:
            // Do nothing, if old value == new value
            if(rec->title == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
           
            break;
        case Mine:
            // Do nothing, if old value == new value
            if(rec->mine == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
           
            break;
        case Address:
            // Do nothing, if old value == new value
            if(rec->address == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
           
            break;
        case Discount:
            // Do nothing, if old value == new value
            if(rec->discount == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
           
            break;
       case Expires:
            // Do nothing, if old value == new value
            if(rec->expires == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
            break;
        case Cert:
            // Do nothing, if old offer == new offer
            if(rec->cert == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
            // Check for duplicates
            else if(lookupEntry(rec->cert) != -1)
            {
                editStatus = DUPLICATE_ENTRY;
                return false;
            }
            break;
        }
        return true;
    }
    return false;
}

QVariant OfferWhitelistTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return columns[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags OfferWhitelistTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return 0;
    Qt::ItemFlags retval = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return retval;
}

QModelIndex OfferWhitelistTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    OfferWhitelistTableEntry *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}

void OfferWhitelistTableModel::updateEntry(const QString &cert, const QString &title, const QString &mine,const QString &address, const QString &expires,const QString &discount, int status)
{
    priv->updateEntry(cert, title, mine, address, expires, discount, status);
}

QString OfferWhitelistTableModel::addRow(const QString &cert, const QString &title, const QString &mine,const QString &address, const QString &expires,const QString &discount)
{
    std::string strCert = cert.toStdString();
    editStatus = OK;
    // Check for duplicate
    {
        if(lookupEntry(cert) != -1)
        {
            editStatus = DUPLICATE_ENTRY;
            return QString();
        }
    }

    // Add entry

    return QString::fromStdString(strCert);
}
void OfferWhitelistTableModel::clear()
{
	beginResetModel();
    priv->cachedEntryTable.clear();
	endResetModel();
}


int OfferWhitelistTableModel::lookupEntry(const QString &cert) const
{
    QModelIndexList lst = match(index(0, Cert, QModelIndex()),
                                Qt::EditRole, cert, 1, Qt::MatchExactly);
    if(lst.isEmpty())
    {
        return -1;
    }
    else
    {
        return lst.at(0).row();
    }
}

void OfferWhitelistTableModel::emitDataChanged(int idx)
{
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, columns.length()-1, QModelIndex()));
}
