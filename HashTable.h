#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "define.h"

enum EM_HASH_STATE
{
    EM_SELECT = 0,
    EM_INSERT,
};

#define DEF_HASH_KEY_SIZE 20           //Ĭ�ϵ�Hash key����

//hash��ṹ
template <class T>
struct _Hash_Table_Cell
{
    char  m_cExists;                       //��ǰ���Ƿ��Ѿ�ʹ��,1�Ѿ�ʹ�ã�0û�б�ʹ��
    char* m_pKey;                          //��ǰ��keyֵ
    int   m_nPosIndex;                     //��ǰ�����ڶ���������±�
    short m_sKeyLen;                       //��ǰ��key����
    T*    m_pValue;                        //��ǰ������ָ��

    _Hash_Table_Cell()
    {
        m_cExists       = 0;
        m_nPosIndex     = 0;
        m_pKey          = NULL;
        m_sKeyLen       = DEF_HASH_KEY_SIZE;
        m_pValue        = NULL;
    }

    //��ʼ������
    void Init(char* pKey, int nKeyLen, int nIndex)
    {
        m_cExists       = 0;
        m_pKey          = pKey;
        m_sKeyLen       = nKeyLen;
        m_pValue        = NULL;
        m_nPosIndex     = nIndex;
    }

    //���������Ϣ
    void Clear()
    {
        m_cExists       = 0;

        if(NULL != m_pKey)
        {
            m_pKey[0] = '\0';
        }

        if(NULL != m_pValue)
        {
            m_pValue = NULL;
        }
    }

    //�õ���ǰ�����С
    int Get_Size(short sKeyLen)
    {
        return sizeof(_Hash_Table_Cell) + sKeyLen * sizeof(char);
    }
};

//hash����ṹ
template <class T>
struct _Hash_Link_Info
{
    int   m_cExists;                          //��ǰ���Ƿ��Ѿ�ʹ��,1�Ѿ�ʹ�ã�0û�б�ʹ��
    int   m_nPosIndex;                        //��ǰ�����ڶ���������±�
    _Hash_Table_Cell<T>* m_pData;             //��ǰ����
    _Hash_Link_Info*     m_pNext;             //��һ������ָ��(��������)
    _Hash_Link_Info*     m_pPerv;             //��һ������ָ��(��������)

    _Hash_Link_Info()
    {
        m_cExists   = 0;
        m_nPosIndex = 0;
        m_pData     = NULL;
        m_pNext     = NULL;
        m_pPerv     = NULL;
    }

    //��ʼ������
    void Init(int nIndex)
    {
        m_cExists   = 0;
        m_nPosIndex = nIndex;
    }

    //���������Ϣ
    void Clear()
    {
        m_cExists   = 0;
        m_pData     = NULL;
        m_pNext     = NULL;
        m_pPerv     = NULL;
    }

    //�õ���ǰ�����С
    int Get_Size()
    {
        return sizeof(_Hash_Link_Info);
    }
};

//Hash����ؽṹ(����Hash���ݶ���ʵ��)
template <class T>
class CHashPool
{
public:
    CHashPool()
    {
        m_pBase      = NULL;
        m_lpTable    = NULL;
        m_nCount     = 0;
        m_nUsedCount = 0;
        m_sKeyLen    = 0;
        m_nCurrIndex = 0;
    }

    ~CHashPool()
    {
        Close();
    }

    int Get_Size(int nHashCount, short sKeySize = DEF_HASH_KEY_SIZE)
    {
        _Hash_Table_Cell<T> objCell;
        int nCellSize = objCell.Get_Size(sKeySize);
        return nCellSize * nHashCount;
    }

    void Init(char* pData, int nHashCount, short sKeySize = DEF_HASH_KEY_SIZE)
    {
        Set_Base_Addr(pData, nHashCount, sKeySize);
    }

    _Hash_Table_Cell<T>* Get_Index(int nIndex)
    {
        return &m_lpTable[nIndex];
    }

    void Close()
    {
        if(NULL != m_pBase)
        {
            //delete[] m_pBase;
            m_pBase   = NULL;
            m_lpTable = NULL;
        }

        m_nCount     = 0;
        m_nUsedCount = 0;
        m_nCurrIndex = 0;
        m_sKeyLen    = 0;
    }

    //����һ����֪���ڴ������(�����ʼ������)
    void Set_Base_Addr(char* pData, int nCount, short sKeySize)
    {
        m_pBase      = pData;
        m_lpTable    = reinterpret_cast<_Hash_Table_Cell<T>*>(pData);
        m_nCount     = nCount;
        m_sKeyLen    = sKeySize;
        m_nCurrIndex = 0;
        int nKeyPool = sizeof(_Hash_Table_Cell<T>) * nCount;

        for(int i = 0; i < m_nCount; i++)
        {
            char* pKey = pData + nKeyPool + i * sKeySize;
            m_lpTable[i].Init(pKey, sKeySize, i);
        }
    }

    //�õ���ǰkey�ĳ���
    short Get_Key_Length()
    {
        return m_sKeyLen;
    }

    //�õ���ǰ������ܸ���
    int Get_Count()
    {
        return m_nCount;
    }

    //�õ�Ŀǰ����ʹ�õĶ������
    int Get_Used_Count()
    {
        return m_nUsedCount;
    }

    //���һ������Ļ����
    _Hash_Table_Cell<T>* Create()
    {
        _Hash_Table_Cell<T>* pCurrCell = NULL;

        if(NULL == m_lpTable || m_nUsedCount == m_nCount)
        {
            return pCurrCell;
        }

        //���ȴӵ�ǰ��������
        for(int i = m_nCurrIndex; i < m_nCount; i++)
        {
            //printf("[CHashPool::Create]i=%d, m_nCurrIndex=%d, m_nCount=%d.\n", i, m_nCurrIndex, m_nCount);
            if(m_lpTable[i].m_cExists == 1)
            {
                continue;
            }
            else
            {
                pCurrCell            = &m_lpTable[i];
                pCurrCell->m_cExists = 1;

                if(i < m_nCount - 1)
                {
                    m_nCurrIndex = i + 1;
                }
                else
                {
                    m_nCurrIndex = 0;
                }

                m_nUsedCount++;
                return pCurrCell;
            }
        }

        //���û�ҵ�����0����ǰ��
        for(int i = 0; i < m_nCurrIndex; i++)
        {
            if(m_lpTable[i].m_cExists == 1)
            {
                continue;
            }
            else
            {
                pCurrCell = &m_lpTable[i];
                pCurrCell->m_cExists = 1;
                m_nCurrIndex = i + 1;
                m_nUsedCount++;
                return pCurrCell;
            }
        }

        //�Ѿ�û�п���
        return pCurrCell;
    }

    //�ͷ�һ������ʹ�õĻ����
    void Delete(_Hash_Table_Cell<T>* pData)
    {
        int nIndex = pData->m_nPosIndex;

        if(nIndex >= 0 && nIndex < m_nCount && pData->m_cExists == 1)
        {
            m_nUsedCount--;
            pData->Clear();
        }
    }

private:
    char*                m_pBase;        //��ǰ�ڴ���ʼλ��
    _Hash_Table_Cell<T>* m_lpTable;      //��ǰHash��������
    int                  m_nCount;       //��ǰHash�������
    int                  m_nUsedCount;   //��ǰ��ʹ�õ�Hash����
    int                  m_nCurrIndex;   //�Ѿ����е��ĵ�ǰHash����λ��
    short                m_sKeyLen;      //��ǰkey�ĳ���
};

//HashTable������
template <class T>
class CHashLinkPool
{
public:
    CHashLinkPool()
    {
        m_pBase      = NULL;
        m_lpTable    = NULL;
        m_nCount     = 0;
        m_nUsedCount = 0;
        m_nCurrIndex = 0;
    }

    ~CHashLinkPool()
    {
        Close();
    }

    int Get_Size(int nHashCount)
    {
        _Hash_Link_Info<T> objLink;
        int nLinkSize = objLink.Get_Size();
        return nLinkSize * nHashCount;
    }

    void Init(char* pData, int nHashCount)
    {
        Set_Base_Addr(pData, nHashCount);
    }

    void Close()
    {
        if(NULL != m_pBase)
        {
            //delete[] m_pBase;
            m_pBase   = NULL;
            m_lpTable = NULL;
        }

        m_nCount     = 0;
        m_nUsedCount = 0;
        m_nCurrIndex = 0;
    }

    //����һ����֪���ڴ������(�����ʼ������)
    void Set_Base_Addr(char* pData, int nCount)
    {
        m_pBase      = pData;
        m_lpTable    = reinterpret_cast<_Hash_Link_Info<T>*>(pData);
        m_nCount     = nCount;
        m_nCurrIndex = 0;

        for(int i = 0; i < m_nCount; i++)
        {
            m_lpTable[i].Init(i);
        }
    }

    //�õ���ǰ������ܸ���
    int Get_Count()
    {
        return m_nCount;
    }

    //�õ�Ŀǰ����ʹ�õĶ������
    int Get_Used_Count()
    {
        return m_nUsedCount;
    }

    //���һ������Ļ����
    _Hash_Link_Info<T>* Create()
    {
        _Hash_Link_Info<T>* pCurrCell = NULL;

        if(NULL == m_lpTable || m_nUsedCount == m_nCount)
        {
            return pCurrCell;
        }

        //���ȴӵ�ǰ��������
        for(int i = m_nCurrIndex; i < m_nCount; i++)
        {
            if(m_lpTable[i].m_cExists == 1)
            {
                continue;
            }
            else
            {
                pCurrCell            = &m_lpTable[i];
                pCurrCell->m_cExists = 1;

                if(i < m_nCount - 1)
                {
                    m_nCurrIndex = i + 1;
                }
                else
                {
                    m_nCurrIndex = 0;
                }

                m_nUsedCount++;
                return pCurrCell;
            }
        }

        //���û�ҵ�����0����ǰ��
        for(int i = 0; i < m_nCurrIndex; i++)
        {
            if(m_lpTable[i].m_cExists == 1)
            {
                continue;
            }
            else
            {
                pCurrCell = &m_lpTable[i];
                pCurrCell->m_cExists = 1;
                m_nCurrIndex = i + 1;
                m_nUsedCount++;
                return pCurrCell;
            }
        }

        //�Ѿ�û�п���
        return pCurrCell;
    }

    //�ͷ�һ������ʹ�õĻ����
    void Delete(_Hash_Link_Info<T>* pData)
    {
        int nIndex = pData->m_nPosIndex;

        if(nIndex >= 0 && nIndex < m_nCount && pData->m_cExists == 1)
        {
            m_nUsedCount--;
            pData->Clear();
        }
    }

private:
    char*                m_pBase;        //��ǰ�ڴ���ʼλ��
    _Hash_Link_Info<T>*  m_lpTable;      //��ǰHash��������
    int                  m_nCount;       //��ǰHash�������
    int                  m_nUsedCount;   //��ǰ��ʹ�õ�Hash����
    int                  m_nCurrIndex;   //�Ѿ����е��ĵ�ǰHash����λ��
};

//hashTable��
template <class T>
class CHashTable
{
public:
    CHashTable()
    {
        m_pBase          = NULL;
        m_lpTable        = NULL;
        m_cIsDelete      = 0;
        m_nCurrLinkIndex = 0;
    }

    ~CHashTable()
    {
        Close();
    }

    //�õ����������ڴ��С
    size_t Get_Size(uint32 u4HashCount, short sKeySize = DEF_HASH_KEY_SIZE)
    {
        return m_objHashPool.Get_Size((int)u4HashCount, sKeySize)
               + m_objHashLinkPool.Get_Size((int)u4HashCount)
               + sizeof(_Hash_Link_Info<T>* ) * u4HashCount;
    }

    //�ر�Hash��
    void Close()
    {
        if(0 == m_cIsDelete)
        {
            delete[] m_pBase;
        }

        m_pBase   = NULL;
        m_lpTable = NULL;
    }

    //��ʼ��Hash��
    void Init(int nHashCount, int nKeySize = DEF_HASH_KEY_SIZE)
    {
        size_t stSize = Get_Size(nHashCount, nKeySize);
        char* pData = new char[stSize];
        memset(pData, 0, stSize);
        int nPos         = 0;
        m_pBase          = pData;
        m_nCurrLinkIndex = 0;
        m_cIsDelete      = 0;
        m_objHashPool.Init(&pData[nPos], nHashCount, nKeySize);
        nPos += m_objHashPool.Get_Size(nHashCount, nKeySize);
        m_objHashLinkPool.Init(&pData[nPos], nHashCount);
        nPos += m_objHashLinkPool.Get_Size(nHashCount);
        m_lpTable = (_Hash_Link_Info<T>** )&pData[nPos];

        for(int i = 0; i < nHashCount; i++)
        {
            m_lpTable[i] = NULL;
        }

        //nPos += sizeof(_Hash_Link_Info<T>* ) * nHashCount;
    }

    //��ʼ��Hash��(�����ڴ��ַ)
    void Init_By_Memory(char* pData, int nHashCount, int nKeySize = DEF_HASH_KEY_SIZE, char cIsDelete = 0)
    {
        memset(pData, 0, Get_Size(nHashCount, nKeySize));
        int nPos         = 0;
        m_pBase          = pData;
        m_nCurrLinkIndex = 0;
        m_cIsDelete      = cIsDelete;
        m_objHashPool.Init(&pData[nPos], nHashCount, nKeySize);
        nPos += m_objHashPool.Get_Size(nHashCount, nKeySize);
        m_objHashLinkPool.Init(&pData[nPos], nHashCount);
        nPos += m_objHashLinkPool.Get_Size(nHashCount);
        m_lpTable = (_Hash_Link_Info<T>** )&pData[nPos];

        for(int i = 0; i < nHashCount; i++)
        {
            m_lpTable[i] = NULL;
        }

        //nPos += sizeof(_Hash_Link_Info<T>* ) * nHashCount;
        //printf("[Init]nPos=%d.\n", nPos);
    }

    //�õ���ǰ��������
    int Get_Count()
    {
        return m_objHashPool.Get_Count();
    }

    //�õ�����������ʹ�õĸ���
    int Get_Used_Count()
    {
        return m_objHashPool.Get_Used_Count();
    }

    //����һ���������е�_Hash_Link_Info<T>* pT
    T* Pop()
    {
        T* pT = NULL;

        if(NULL == m_lpTable)
        {
            //û���ҵ������ڴ�
            return pT;
        }

        //Ѱ��һ�������õĶ��󣬵�������
        for(int i = m_nCurrLinkIndex; i < m_objHashPool.Get_Count(); i++)
        {
            if(m_lpTable[i] != NULL)
            {
                //ȡ����ǰ������
                pT         = m_lpTable[i]->m_pData->m_pValue;
                char* pKey = m_lpTable[i]->m_pData->m_pKey;

                //printf("[Pop]1 pKey=%s.\n", pKey);

                //��������
                Del_Hash_Data(pKey);
                return pT;
            }
        }

        for(int i = 0; i < m_nCurrLinkIndex; i++)
        {
            if(m_lpTable[i] != NULL)
            {
                //ȡ����ǰ������
                pT = m_lpTable[i]->m_pData->m_pValue;
                char* pKey = m_lpTable[i]->m_pData->m_pKey;

                //����״̬
                m_lpTable[i] = m_lpTable[i]->m_pNext;
                m_nCurrLinkIndex = i;

                //printf("[Pop]2 pKey=%s.\n", pKey);

                //��������
                Del_Hash_Data(pKey);
                return pT;
            }
        }

        return pT;
    }

    //��һ���Ѿ�ʹ����ɵĶ��󣬷Żص�����
    bool Push(const char* pKey, T* pT)
    {
        if(NULL == m_lpTable)
        {
            //û���ҵ������ڴ�
            return false;
        }

        //printf("[Push]pKey=%s.\n", pKey);

        //����key����������hashλ��
        unsigned long uHashStart = HashString(pKey, m_objHashPool.Get_Count());

        _Hash_Link_Info<T>* pLastLink = m_lpTable[uHashStart];

        while(NULL != pLastLink)
        {
            pLastLink = pLastLink->m_pNext;
        }

        //�Ӷ�����л�ȡһ���¶���
        _Hash_Table_Cell<T>* pData = m_objHashPool.Create();

        if(NULL == pData)
        {
            return false;
        }

        _Hash_Link_Info<T>* pLink = m_objHashLinkPool.Create();

        if(NULL == pLink)
        {
            return false;
        }

        sprintf_safe(pData->m_pKey, pData->m_sKeyLen, "%s", pKey);
        pData->m_pValue    = pT;
        pLink->m_pData     = pData;
        pLink->m_pPerv     = pLastLink;

        if(pLastLink != NULL)
        {
            pLastLink->m_pNext = pLink;
        }
        else
        {
            m_lpTable[uHashStart] = pLink;
        }

        return true;
    }

    //�õ���������ʹ�õ�ָ��
    void Get_All_Used(vector<T*>& vecList)
    {
        if(NULL == m_lpTable)
        {
            return;
        }

        vecList.clear();

        for(int i = 0; i < m_objHashPool.Get_Count(); i++)
        {
            if(NULL != m_lpTable[i])
            {
                _Hash_Link_Info<T>* pLastLink = m_lpTable[i];

                while(NULL != pLastLink)
                {
                    vecList.push_back(pLastLink->m_pData->m_pValue);
                    pLastLink = pLastLink->m_pNext;
                }
            }
        }
    }

    //����ǰHash�����е����ж���ָ��
    void Clear()
    {
        vector<string> vecList;

        if (NULL == m_lpTable)
        {
            return;
        }

        for (int i = 0; i < m_objHashPool.Get_Count(); i++)
        {
            if (NULL != m_lpTable[i])
            {
                _Hash_Link_Info<T>* pLastLink = m_lpTable[i];

                while (NULL != pLastLink)
                {
                    vecList.push_back(pLastLink->m_pData->m_pKey);
                    pLastLink = pLastLink->m_pNext;
                }
            }
        }

        //��������
        for (int i = 0; i < (int)vecList.size(); i++)
        {
            Del_Hash_Data(vecList[i].c_str());
        }
    }

    //���һ��Hash���ݿ�
    int Add_Hash_Data(const char* pKey, T* pValue)
    {
        if(NULL == m_lpTable)
        {
            //û���ҵ������ڴ�
            return -1;
        }

        if((short)strlen(pKey) >= m_objHashPool.Get_Key_Length())
        {
            //��ǰkey�ĳ��ȳ�����HashTable��key���ȡ�
            return -1;
        }

        int nPos = GetHashTablePos(pKey, EM_INSERT);

        if(-1 == nPos)
        {
            //�ڴ����������������Ѿ�����
            return -1;
        }
        else
        {
            if(NULL == m_lpTable[nPos])
            {
                //�Ӷ�����л�ȡһ���¶���
                _Hash_Table_Cell<T>* pData = m_objHashPool.Create();

                if(NULL == pData)
                {
                    return -1;
                }

                _Hash_Link_Info<T>* pLink = m_objHashLinkPool.Create();

                if(NULL == pLink)
                {
                    return -1;
                }
                else
                {
                    sprintf_safe(pData->m_pKey, pData->m_sKeyLen, "%s", pKey);
                    pData->m_pValue = pValue;
                    pLink->m_pData = pData;
                    m_lpTable[nPos] = pLink;
                }
            }
            else
            {
                //����Ѿ����ڶ������ҵ���ǰ���������һ�������֮
                _Hash_Link_Info<T>* pLastLink = m_lpTable[nPos];

                while(NULL != pLastLink)
                {
                    if(pLastLink->m_pNext == NULL)
                    {
                        break;
                    }

                    pLastLink = pLastLink->m_pNext;
                }

                //�Ӷ�����л�ȡһ���¶���
                _Hash_Table_Cell<T>* pData = m_objHashPool.Create();

                if(NULL == pData)
                {
                    return -1;
                }

                _Hash_Link_Info<T>* pLink = m_objHashLinkPool.Create();

                if(NULL == pLink)
                {
                    return -1;
                }
                else
                {
                    sprintf_safe(pData->m_pKey, pData->m_sKeyLen, "%s", pKey);
                    pData->m_pValue = pValue;
                    pLink->m_pData = pData;
                    pLink->m_pPerv = pLastLink;

                    if (NULL != pLastLink)
                    {
                        pLastLink->m_pNext = pLink;
                    }
                }
            }

            return nPos;
        }
    }

    //���һ������ӳ���Ӧ��ֵ
    T* Get_Hash_Box_Data(const char* pKey)
    {
        if(NULL == m_lpTable)
        {
            //û���ҵ������ڴ�
            return NULL;
        }

        int nPos = GetHashTablePos(pKey, EM_SELECT);

        if(-1 == nPos)
        {
            //û���ҵ�
            return NULL;
        }
        else
        {
            T* pT = NULL;
            _Hash_Link_Info<T>* pLastLink = m_lpTable[nPos];

            while(NULL != pLastLink)
            {
                if(NULL != pLastLink->m_pData && strcmp(pLastLink->m_pData->m_pKey, pKey) == 0)
                {
                    pT = pLastLink->m_pData->m_pValue;
                }

                if(pLastLink->m_pNext == NULL)
                {
                    break;
                }

                pLastLink = pLastLink->m_pNext;
            }

            return pT;
        }
    }

    //����һ��hash���ݿ�
    int Del_Hash_Data(const char* pKey)
    {
        return DelHashTablePos(pKey);
    }

private:
    //����key��Ӧ��hash��ֵ
    unsigned long HashString(const char* pKey, int nCount)
    {
        unsigned char* pukey = (unsigned char*)pKey;
        unsigned int seed = 131; /* 31 131 1313 13131 131313 etc..*/
        unsigned int h = 0;

        while (*pukey)
        {
            h = h * seed + (*pukey++);
        }

        h = h & 0x7FFFFFFF;

        if(0 == nCount)
        {
            return h;
        }
        else
        {
            return h % nCount;
        }
    }

    //�õ�hashָ����λ��
    int GetHashTablePos(const char* lpszString, EM_HASH_STATE emHashState)
    {
        unsigned long uHashStart = HashString(lpszString, m_objHashPool.Get_Count());

        //��ȡ�������ȶ�
        if(NULL == m_lpTable[uHashStart])
        {
            if(EM_INSERT == emHashState)
            {
                return uHashStart;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            _Hash_Link_Info<T>* pLastLink = m_lpTable[uHashStart];

            while(NULL != pLastLink)
            {
                //printf("[CHashTable::GetHashTablePos]pLastLink->m_pData=0x%08x.\n", pLastLink->m_pData);
                //printf("[CHashTable::GetHashTablePos]pLastLink->m_pData->m_pKey=%s.\n", pLastLink->m_pData->m_pKey);
                //printf("[CHashTable::GetHashTablePos]lpszString=%s.\n", lpszString);
                if(NULL != pLastLink->m_pData && strcmp(pLastLink->m_pData->m_pKey, lpszString) == 0)
                {
                    //�ҵ��˶�Ӧ��key,��������Ѿ�����
                    if(EM_INSERT == emHashState)
                    {
                        return -1;
                    }
                    else
                    {
                        return uHashStart;
                    }
                }

                pLastLink = pLastLink->m_pNext;
            }

            return uHashStart;
        }

    }

    //ɾ��ָ��������
    int DelHashTablePos(const char* lpszString)
    {
        unsigned long uHashStart = HashString(lpszString, m_objHashPool.Get_Count());

        //��ȡ�������ȶ�
        if(NULL == m_lpTable[uHashStart])
        {
            return -1;
        }
        else
        {
            _Hash_Link_Info<T>* pLastLink = m_lpTable[uHashStart];

            while(NULL != pLastLink)
            {
                if(NULL != pLastLink->m_pData && strcmp(pLastLink->m_pData->m_pKey, lpszString) == 0)
                {
                    //�ҵ��˶�Ӧ��key,��������Ѿ�����
                    if(pLastLink->m_pPerv == NULL)
                    {
                        m_lpTable[uHashStart] = pLastLink->m_pNext;

                        if(NULL != pLastLink->m_pNext)
                        {
                            pLastLink->m_pNext->m_pPerv = NULL;
                        }
                    }
                    else
                    {
                        pLastLink->m_pPerv->m_pNext = pLastLink->m_pNext;

                        if(NULL != pLastLink->m_pNext)
                        {
                            pLastLink->m_pNext->m_pPerv = pLastLink->m_pPerv;
                        }
                    }

                    //����ָ������Ͷ���
                    m_objHashPool.Delete(pLastLink->m_pData);
                    m_objHashLinkPool.Delete(pLastLink);
                    return 0;
                }

                pLastLink = pLastLink->m_pNext;
            }
        }

        return 0;
    }

private:
    CHashPool<T>         m_objHashPool;      //Hash�����
    CHashLinkPool<T>     m_objHashLinkPool;  //Hash��������
    _Hash_Link_Info<T>** m_lpTable;          //��ǰHash��������
    int                  m_nCurrLinkIndex;   //��ǰ����λ��
    char*                m_pBase;            //�ڴ��Ļ�����ַ
    char                 m_cIsDelete;        //��ǰ��������ʱ���Ƿ�����ڴ棬0�ǻ��գ�1�ǲ����ա�
};


#endif
