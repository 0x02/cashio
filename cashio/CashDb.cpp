#include "CashDb.h"
#include <cstdio>
using namespace std;
#include "DbDef.h"

const int SQL_BUF_SIZE = 1024;

CashDb::CashDb()
{
    mSqlBuf = new char[SQL_BUF_SIZE];
    mSqlBufSize = SQL_BUF_SIZE;
}

CashDb::~CashDb()
{
    if (!mDbName.empty())
    {
        CloseDb();
    }

    delete mSqlBuf;
}

void CashDb::OpenDb(const string &path)
{
    mDbName = path;
    sqlite3_open(mDbName.c_str(), &mDbConn);
    SetupPragma();
}

void CashDb::CloseDb()
{
    mDbName.clear();
    sqlite3_close(mDbConn);
    mDbConn = NULL;
}

void CashDb::InitDb()
{
    // tags table
    ExecSql(SQL_CREATE_TABLE_TAGS);
    // account table
    ExecSql(SQL_CREATE_TABLE_ACCOUNT);
    // account-tag table
    ExecSql(SQL_CREATE_TABLE_ACCOUNT_TAG);
}

void CashDb::ClearDb()
{
    // TODO: detect all tables, then remove them
    FORMAT_SQL(SQL_DROP_TABLE, TABLE_TAGS);
    ExecSql();
    FORMAT_SQL(SQL_DROP_TABLE, TABLE_ACCOUNT);
    ExecSql();
    FORMAT_SQL(SQL_DROP_TABLE, TABLE_ACCOUNT_TAG);
    ExecSql();
}

void CashDb::InsertTag(const Tag &tag)
{
    FORMAT_SQL(SQL_INSERT_TAG, tag.name.c_str(), tag.color);
    ExecSql();
}

void CashDb::DropTag(const string &tagName)
{

}

void CashDb::UpdateTag(const string &tagName, const Tag &newTag)
{

}

void CashDb::GetTags(TagVector &tags)
{
    tags.clear();
    COPY_SQL(SQL_QUERY_TAGS_ALL);
    Prepare();
    while (NextStep() == StepRow)
    {
        Tag tag(ColumnString(0),
                ColumnInt(1));
        tags.push_back(tag);
    }
}

bool CashDb::HasTag(const string &tagName)
{
    FORMAT_SQL(SQL_QUERY_TAGS_HAS_TAG, tagName.c_str());
    Prepare();
    return (NextStep() == StepRow) ? true : false;
}

void CashDb::InsertRow(const string& uuid, const Row* row)
{    
    // insert into account
    FORMAT_SQL(SQL_INSERT_ACCOUNT,
               uuid.c_str(),
               row->date.c_str(),
               row->io.c_str(),
               row->amount,
               row->note.c_str());
    ExecSql();

    // complete tags and relationships
    for (size_t i = 0; i < row->tags.size(); ++i)
    {
        Tag tag = row->tags[i];
        if (!HasTag(tag.name))
            InsertTag(tag);

        FORMAT_SQL(SQL_INSERT_ACCOUNT_TAG, row->date.c_str(), tag.name.c_str());
        ExecSql();
    }
}

void CashDb::DeleteRow(const string &uuid)
{
    FORMAT_SQL(SQL_DELETE_ACCOUNT, uuid.c_str());
    ExecSql();
}

void CashDb::UpdateRow(const string &uuid, const Row &row, const UuidVector& tagUuids)
{
    // clear all old tags
    FORMAT_SQL(SQL_DELETE_ACCOUNT_TAGS, uuid.c_str());
    ExecSql();

    // update account
    FORMAT_SQL(SQL_UPDATE_ACCOUNT,
               row.date.c_str(),
               row.io.c_str(),
               row.amount,
               row.note.c_str(),
               uuid.c_str());
    ExecSql();

    // complete tags and relationships
    COPY_SQL(SQL_BEGIN);
    ExecSql();
    for (size_t i = 0; i < row.tags.size(); ++i)
    {
        Tag tag = row.tags[i];
        if (!HasTag(tag.name))
            InsertTag(tag);

        FORMAT_SQL(SQL_INSERT_ACCOUNT_TAG, tagUuids[i].c_str(), uuid.c_str(), tag.name.c_str());
        ExecSql();
    }
    COPY_SQL(SQL_COMMIT);
    ExecSql();
}

void CashDb::QueryAllRows(UuidVector& range)
{
    QueryRows(SQL_QUERY_ACOUNT_ALL_DATE, range);
}

void CashDb::QueryRows(const string& query, UuidVector& range)
{
    range.clear();
    COPY_SQL(query.c_str());
    Prepare();
    while (NextStep() == StepRow)
    {
        range.push_back(ColumnString(0));
    }
}

void CashDb::GetRows(const UuidVector& range, RowPtrVector &rows)
{
    for (size_t i = 0; i < rows.size(); ++i)
    {
        delete rows[i];
    }
    rows.clear();

    for (size_t i = 0; i < range.size(); ++i)
    {
        FORMAT_SQL(SQL_QUERY_ACCOUNT_ROW, range[i].c_str());
        Prepare();
        if (NextStep() != StepRow) return;
        Row* row = new Row;
        row->date = ColumnString(0);
        row->io = ColumnString(1);
        row->amount = ColumnDouble(2);
        row->note = ColumnString(3);

        FORMAT_SQL(SQL_QUERY_ACCOUNT_TAGS, range[i].c_str());
        Prepare();
        row->tags.clear();
        while (NextStep() == StepRow)
        {
            Tag tag;
            tag.name = ColumnString(0);
            tag.color = ColumnInt(1);
            row->tags.push_back(tag);
        }

        rows.push_back(row);
    }
}

bool CashDb::HasRow(const string &uuid)
{
    FORMAT_SQL(SQL_QUERY_ACCOUNT_HAS_UUID, uuid.c_str());
    Prepare();
    return (NextStep() == StepRow) ? true : false;
}

string CashDb::GetTime()
{
    Prepare(SQL_QUERY_DATE);
    return (NextStep() == StepRow) ? ColumnString(0) : "";
}
