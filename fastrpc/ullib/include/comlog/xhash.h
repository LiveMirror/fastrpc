/**
 * @file    hash_table.h
 * @author  肖伟
 * @date    2006-08-02
 * @version $Revision: 1.2 $
 * @brief   使用模板实现hash_table 适用于所有基本类型和C风格的struct,不支持class
 * */

#ifndef _XHASH_H
#define _XHASH_H

/**
 *没有收集删除单元内存，在多读一写的情况下多线程安全
 *收集删除单元的内存，加了读写锁，多线程安全
 */

/**
 * @brief 默认情况下线程不安全，不使用读写锁操作内存，程序仅仅保证在多读一写内存的情况下是安全的。而且对删除的单元不回收利用。
 * 		  如果定义了_HASH_THREAD_SAFE 那么使用读写锁进行内存操作，但是速度会比不使用读写锁的慢很多，N个线程慢2N倍左右。对删除单元采取空间回收利用机制。
 **/

//#define _HASH_THREAD_SAFE

#ifdef HASH_THREAD_SAFE
#define _HASH_THREAD_SAFE
#endif

#if 0
#ifdef _HASH_THREAD_SAFE
#define _XOPEN_SOURCE 500
#endif
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ul_log.h>
#include <ul_func.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <pthread.h>

#define _XHASH_WARNING_LOG(fmt, arg...) do { \
	    ul_writelog(UL_LOG_WARNING, "[0][%s:%d]" fmt, __FILE__, __LINE__, ## arg); \
} while (0)

#define _XHASH_NOTICE_LOG(fmt, arg...) do { \
	    ul_writelog(UL_LOG_NOTICE, fmt, ## arg); \
} while (0)


namespace comspace
{
enum{
	HASH_SUCCESS=0,
	HASH_VAL_EXIST,
	HASH_VAL_NO_EXIST,
	HASH_ACLLOMEM_ERROR,
};

/**
 * @class hash_map
 * @brief 一个模板类，有两个模板参数：key_t哈希表健值类型，value_t哈希表数值类型
 * */

template<class key_t>
int _hash_compare_fun( const key_t &k1, const key_t &k2 )
{
    return (k1==k2)?0:1;
}

template <class key_t, class value_t>
class hash_map
{
private:
	//哈希函数类型
	typedef unsigned int (*hash_fcn_t)(const key_t &);
	typedef int (*compare_t)(const key_t &, const key_t &);
	
	//fix by xiaowei again
	static const int BLOCK_SIZE = 1024;
	static const int BUFFER_SIZE = 4 * 1024;
	/**
	 * @struct node_t 
	 * @brief 哈希表结点类型，内部包含一个键值和一个数值以及一个指向下个结点的指针
	 * */
	typedef struct _node_t
	{
		key_t key;
		value_t value;
		struct _node_t * next;
	}node_t;
	
	/**
	 * @struct bucket_t
	 * @brief 哈希表桶结构，内部包含这个桶里第一个结点的指针
	 * */
	typedef struct _bucket_t
	{
#ifdef HASH_MONITOR_X
	    	int m_monitor;
#endif
		node_t * first;
	}bucket_t;
	
	/**
	 * @struct block_t
	 * @brief 结点块类型，内部包含了大块的内存，用于分配给哈希表使用，以及指向下一个块的指针
	 * */
	typedef struct _block_t
	{
		node_t node_block[BLOCK_SIZE];
		struct _block_t * next;
	}block_t;

	typedef struct _buffer_node
	{
		key_t key;
		value_t value;
	}buffer_node;

public:
	hash_map()
	{
		setAllZero();
	}
	/**
	 * @brief 析构函数,即使释放内存
	 */
	~hash_map()
	{
		clear();
	}


	/**
	 * @brief 根据键值查找数据
	 *
	 * @param key_t 键值
	 * @param value_t 存储返回的数据
	 *
	 * @return 	成功返回 0
	 * 			失败返回 HASH_VAL_NO_EXIST
	 **/
	int get( const key_t &k, value_t *val=NULL )
	{
		if( !( m_bucket&&m_first_block ) ){
			return -1;
		}

#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_rdlock(m_lock);
#endif
		int ret = HASH_VAL_NO_EXIST;

		unsigned int key = getHashKey( k );
		node_t * ptr = m_bucket[key].first;

		while( ptr ){
			if( /*ptr->key == k*/ m_equal( ptr->key, k )==0 ){
				if(val)
					*val = ptr->value;
				ret = 0;
				break;
			}else{
				ptr = ptr->next;
			}
		}

#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
		return ret;
	}

	/**
	 * @brief 根据键值在hash中插入数据
	 * @param key_t  键值
	 * @param value_t 要插入hash的数据
	 * @param int 默认值等于0，表示如果值存在不覆盖，如果非零，覆盖原来的值。
	 *
	 * @return 插入或者替换成功成功返回: 0
	 * 			无法分配空间返回: HASH_ACLLOMEM_ERROR
	 * 			已经存在该值返回: HASH_VAL_EXIST
	 **/
	int set( const key_t &k, const value_t &val, int flag = 0 )
	{
		if( !( m_bucket&&m_first_block ) ){
			return -1;
		}
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_wrlock(m_lock);
#endif
		int ret = 0;

		unsigned int key = getHashKey( k );
		node_t *ptr = m_bucket[key].first;
		node_t *front=0;
		node_t *node=0;

		//是否已经存在
		while( ptr ){
			if( /*ptr->key == k*/ m_equal( ptr->key, k )==0 ){
				if( flag ){
					ptr->value = val;
					ret = 0;
					goto _SET_END;
				}
				ret = HASH_VAL_EXIST;
				goto _SET_END;
			}else{
				front = ptr;
				ptr = ptr->next;
			}
		}

		//分配空间
		node = getNodeSpace();
		if( node == 0 ){//分配空间失败
			_XHASH_WARNING_LOG("[HASH] malloc memory error");
			ret = HASH_ACLLOMEM_ERROR;
			goto _SET_END;
		}

		node->key = k;
		node->value = val;
		node->next = 0;

		if( front ) front->next = node;
		else{
#ifdef HASH_MONITOR_X
		    used_bucket ++;
#endif
		    m_bucket[key].first = node;
		}

		m_node_num++;
#ifdef HASH_MONITOR_X
		m_bucket[key].m_monitor ++;
#endif

_SET_END:
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
		return ret;
	}

	/**
	 * @brief 重HASH中删除数据
	 * @param key_t  键值
	 *
	 * @return 	删除成功返回: 0
	 * 			元素不存在返回: HASH_VAL_NO_EXIST
	 **/
	int erase(const key_t &k)
	{
		if( !( m_bucket&&m_first_block ) ){
			return -1;
		}
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_wrlock(m_lock);
#endif
		int ret= HASH_VAL_NO_EXIST;

		unsigned int key = getHashKey( k );
		node_t *ptr = m_bucket[key].first;
		node_t *front = 0;

		while( ptr ){
			if( /*ptr->key == k*/ m_equal(ptr->key,k)==0 ){
				if( front==0 ){
					m_bucket[key].first = ptr->next;
#ifdef HASH_MONITOR_X
					used_bucket--;
#endif
				}else{
					front->next = ptr->next;
				}
#ifdef _HASH_THREAD_SAFE
				ptr->next = m_free_node;
				m_free_node = ptr;
#endif
				
				m_node_num--;				
#ifdef HASH_MONITOR_X
				m_bucket[key].m_monitor--;
#endif
				ret = 0;
				goto _ERASE_END;
			}
			front = ptr;
			ptr = ptr->next;
		}

_ERASE_END:
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
		return ret;
	}

	/**
	 * @brief 获取HASH的数据量
	 *
	 * @return 返回hash中的数据数目
	 **/
	inline int size() const{
		return m_node_num;
	}


	/**
	 * @brief 将HASH里面的数据写入磁盘
	 * @note  本函数没有做大文件处理,在32位平台上,数据量不可大于2G
	 * @param char  文件名
	 *
	 *
	 * @return 成功返回0，失败-1
	 **/
	int savedata(const char *fn){
		if( !( m_bucket&&m_first_block ) ){
			return -1;
		}

		//buffer_node *buffer = new buffer_node[BUFFER_SIZE];
		buffer_node *buffer = (buffer_node *) malloc( sizeof(buffer_node)*BUFFER_SIZE );
		if( buffer==0 ){
			_XHASH_WARNING_LOG("can't malloc memory");
			return -1;
		}
		int bufcount=0;

		node_t *ptr= 0;
		int xcount = 0;
		int fd = open( fn, O_WRONLY|O_CREAT|O_TRUNC, 0666 );
		if( fd == -1 ){
			_XHASH_WARNING_LOG("can't open [%s] file",fn);
			return -1;
		}

#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_rdlock(m_lock);
#endif

		unsigned int sizekey = sizeof( key_t );
		unsigned int sizeval = sizeof( value_t );

		if( write( fd, &sizekey, sizeof(unsigned int) ) != sizeof(unsigned int) ){
			goto _WRITE_ERROR;
		}
		if( write( fd, &sizeval, sizeof(unsigned int) ) != sizeof(unsigned int) ){
			goto _WRITE_ERROR;
		}

		for( int i=0; i<m_bucket_num; i++ ){
			ptr = m_bucket[i].first;
			while( ptr ){
				
				buffer[bufcount].key = ptr->key;
				buffer[bufcount].value = ptr->value;
				bufcount++;

				if( bufcount==BUFFER_SIZE ){
					if( write( fd, buffer, BUFFER_SIZE*sizeof(buffer_node) ) != (int)sizeof(buffer_node)*(int)BUFFER_SIZE )
						goto _WRITE_ERROR;
					xcount += BUFFER_SIZE;
					bufcount = 0;
				}
				
				ptr = ptr->next;
			}
		}

		if( bufcount>0 ){
			if( write( fd, buffer, bufcount*sizeof(buffer_node) ) != (int)(sizeof(buffer_node)*bufcount) )
				goto _WRITE_ERROR;
			xcount += bufcount;
		}
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
		_XHASH_NOTICE_LOG("save [%d] data to [%s] file success",xcount,fn);
		close(fd);
		free(buffer);
		return 0;

_WRITE_ERROR:
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
		_XHASH_WARNING_LOG("[HASH] Write File[%s] error",fn);
		close(fd);
		free(buffer);
		return -1;
	}


	/**
	 * @brief 从磁盘中读取信息追加到hash中
	 * @param char  文件名
	 *
	 * @return 成功返回0,失败-1.
	 **/
	int loaddata(const char *fn, int flag = 0 ){
		if( !( m_bucket&&m_first_block ) ){
			return -1;
		}
		buffer_node *buffer = (buffer_node *)malloc(sizeof(buffer_node)*BUFFER_SIZE);
		if( buffer==0 ){
			_XHASH_WARNING_LOG("can't malloc memory");
			return -1;
		}
		int bufcount;

		unsigned int sizekey,sizeval;
		int xcount = 0;

		int fd = open( fn, O_RDONLY );
		if( fd == -1 ){
			_XHASH_WARNING_LOG("can't open [%s] file",fn);
			return -1;
		}
		if( read( fd, &sizekey, sizeof(unsigned int) ) != sizeof(unsigned int) )
			goto _READ_ERROR;
		if( sizekey != sizeof(key_t) )
			goto _READ_ERROR;
		if( read( fd, &sizeval, sizeof(unsigned int) ) != sizeof(unsigned int) )
			goto _READ_ERROR;
		if( sizeval != sizeof( value_t ) )
			goto _READ_ERROR;

		for(;;){
			bufcount = read( fd, buffer, sizeof(buffer_node)*BUFFER_SIZE );
			bufcount /= sizeof(buffer_node);
			if( bufcount<=0 )break;
			for( int i=0; i<bufcount; i++ ){
				if( set( buffer[i].key, buffer[i].value, flag ) == 0 )
					xcount++;
			}
			if( bufcount<BUFFER_SIZE ) break;
		}
		
		_XHASH_NOTICE_LOG("load [%d] data from [%s] file success",xcount,fn);
		close(fd);
		free(buffer);
		return 0;

_READ_ERROR:
		_XHASH_WARNING_LOG("read error [%s] file",fn);
		close(fd);
		free(buffer);
		return -1;
	}


	/**
	 * @brief 遍历hash表中的值
	 * @param fun  对hash表中每个值的处理函数
	 *
	 **/
	void list( void (*fun)(const value_t &t, void *param), void *paramv ){
		if( !( m_bucket&&m_first_block ) || !fun){
			return;
		}

#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_rdlock(m_lock);
#endif
		node_t *tmp = 0;
		for( int i=0; i<m_bucket_num; i++ ){
			tmp = m_bucket[i].first;
			while( tmp ){
				fun( tmp->value, paramv );
				tmp = tmp->next;
			}
		}
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
	}

	/**
	 * @brief 重置hash表，将hash表清空，但是分配的内存不回收
	 *
	 **/
	inline void reset()
	{
		if( !( m_bucket&&m_first_block ) ){
			return -1;
		}
#ifdef HASH_MONITOR_X
		used_bucket = 0;
#endif
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_wrlock(m_lock);
#endif	
		m_node_num = 0;
		m_free_num = BLOCK_SIZE;
		m_cur_block = m_first_block;
#ifdef _HASH_THREAD_SAFE
		m_free_node = 0;
#endif
		memset( m_bucket , 0, sizeof(bucket_t)*m_bucket_num );
#ifdef _HASH_THREAD_SAFE
		pthread_rwlock_unlock(m_lock);
#endif
	}

	//测试用的函数
#ifdef _HASH_MAP_TEST
	void walk()
	{
		node_t *tmp=0;
		for( int i=0; i< m_bucket_num; i++ ){
			tmp = m_bucket[i].first;
			while( tmp ){
				cout<<"["<<getHashKey(tmp->key)<<":"<<tmp->key<<":"<<tmp->value<<"]"<<endl;
				tmp = tmp->next;
			}
		}
	}
	void walk_block(int flag=0)
	{
		block_t *head = m_first_block;
		int xcount=0;

		for( ;head; ){
			if( flag ){
				int start=BLOCK_SIZE-1;
				int end = 0;

				for( ;start>=end;start--){
					node_t *tmp = &head->node_block[start];
					cout<<"["<<getHashKey(tmp->key)<<":"<<tmp->key<<":"<<tmp->value<<"]" << endl;
					if(--flag<1)break;
				}
			}

			head = head->next;
			xcount++;
		}

		cout<<xcount<<endl;


	}
	void walk_free()
	{
#ifdef _HASH_THREAD_SAFE
		node_t *head = m_free_node;
		while( head ){
			cout<<"["<<getHashKey(head->key)<<":"<<head->key<<":"<<head->value<<"]"<<endl;
			head = head->next;
		}
#else
		cout<<"no gather memory"<<endl;
#endif
	}
#endif
	
private:
	
	/**
	 * @brief 类成员变量清零
	 *
	 */
	inline void setAllZero()
	{
		m_bucket_num = m_node_num = m_free_num = 0;
		m_bucket = NULL;
		m_cur_block = NULL;
#ifdef _HASH_THREAD_SAFE
		m_free_node = NULL;
		m_lock = NULL;
#endif
		m_first_block = NULL;
#ifdef HASH_MONITOR_X
		used_bucket = 0;
#endif
	}

	/**
	 * @brief 根据键值，重新得到hash值，主要防止键值超出bucket的最大范围
	 *
	 * @return hash key
	 **/
	inline unsigned int getHashKey( const key_t &val )
	{
		//防治错误的key产生
		return m_fcn(val)%(unsigned int)m_bucket_num;
	}
public:
	/**
	 * @brief 初始化HASH表的内存
	 *
	 * @param unsigned int  Hash表内bucket的大小
	 *
	 * @return 成功返回0，错误返回-1
	 **/
	int create(int hn, hash_fcn_t fcn, compare_t cft = _hash_compare_fun )
	{
		if (hn <= 0) {
			_XHASH_WARNING_LOG("invalid input hn[%d] <=0", hn);
			return -1;
		}

		clear();//防止内存泄露

#ifdef _HASH_THREAD_SAFE
		if( ( m_lock = ( pthread_rwlock_t * )malloc( sizeof( pthread_rwlock_t ) ) ) == 0 ){
			_XHASH_WARNING_LOG("malloc lock error");
			return -1;
		}
		if( pthread_rwlock_init( m_lock, NULL ) != 0 ){
			_XHASH_WARNING_LOG("init rwlock fail");
			free(m_lock);
			m_lock = 0;
			return -1;
		}
#endif
		if( hn ==0 || fcn == NULL || cft==NULL){
			_XHASH_WARNING_LOG("unvalid hash_num[%d] or unvalid hash or compare function", hn);
#ifdef _HASH_THREAD_SAFE
			free(m_lock);
			m_lock = 0;
#endif
			return -1;
		}
		m_fcn = fcn;//设置Hash函数
		m_equal = cft;
		m_bucket_num = hn;

		if( ( m_bucket = (bucket_t * )malloc( sizeof(bucket_t)*m_bucket_num ) ) == 0 ){
			_XHASH_WARNING_LOG(" [HASH ERROR] Fail to new a hash bucket table [%zu].",
					m_bucket_num * sizeof( bucket_t )  );
#ifdef _HASH_THREAD_SAFE
			free(m_lock);
			m_lock = 0;
#endif
			return -1;
		}
		memset( m_bucket, 0, sizeof( bucket_t ) * m_bucket_num );
		
		if( ( m_first_block = ( block_t * ) malloc( sizeof(block_t) ) ) == 0 ){
			_XHASH_WARNING_LOG("[HASH ERROR] Fail to new a hash block [%zu]",
					sizeof( block_t ) );
#ifdef _HASH_THREAD_SAFE
			free(m_lock);
			m_lock = 0;
#endif
			free( m_bucket );
			m_bucket = 0;
			return -1;
		}
		m_first_block->next = 0;//表示结尾
	
		m_cur_block = m_first_block;
		m_free_num = BLOCK_SIZE;
		
		return 0;		
	}

	/**
	 * @brief 删除hash表
	 *
	 **/
	void clear()
	{
		if( m_bucket ){
			free( m_bucket );
		}
		
		//free block
		block_t *tmp;
		while( m_first_block ){
			tmp = m_first_block;
			m_first_block = m_first_block->next;
			free(tmp);
		}
#ifdef _HASH_THREAD_SAFE
		if( m_lock ){
			pthread_rwlock_destroy(m_lock);
			free(m_lock);
		}
#endif

		setAllZero();
	}

private:
	/**
	 * @brief 为新加入的节点申请内存空间
	 *
	 * @return 返回空间地址，错误返回NULL
	 **/
	inline node_t * getNodeSpace()
	{
#ifdef _HASH_THREAD_SAFE
		if( m_free_node ){//free空间内有空间
			node_t *tmp = m_free_node;
			m_free_node = m_free_node->next;
			return tmp;
		}else{
#endif
			if( m_free_num==0 ){
				if( m_cur_block->next ){
					m_cur_block = m_cur_block->next;
				}else{
					block_t *btmp = (block_t *)malloc( sizeof(block_t) );
					if( btmp==0 ){
						_XHASH_WARNING_LOG("[HASH ERROR] Fail to new a Hash Block[%zu]",sizeof(block_t));
						return NULL;
					}
					btmp->next = 0;
					m_cur_block->next = btmp;
					m_cur_block = btmp;
				}
				m_free_num = BLOCK_SIZE;
			}

			return &( m_cur_block->node_block[ --m_free_num ] );
#ifdef _HASH_THREAD_SAFE
		}

		return NULL;
#endif
	}
public:
#ifdef HASH_MONITOR_X
	void monitor(){
	    _XHASH_NOTICE_LOG("[monitor]--------------------");
	    _XHASH_NOTICE_LOG("[monitor]bucket[%d/%d]",
		    used_bucket,
		    m_bucket_num);
	    _XHASH_NOTICE_LOG("[monitor]nodes[%d]",
		    m_node_num);
	    _XHASH_NOTICE_LOG("[monitor]--------------------");
	}
	void monitor(int *used, int *total, int *nodes){
	    *used = used_bucket;
	    *total = m_bucket_num;
	    *nodes = m_node_num;
	}
	int monitor_moreThan(int n){
	    int ret=0;
	    for(int i=0;i<m_bucket_num;i++)
		if( m_bucket[i].m_monitor > n )
		    ret ++;
	    return ret;
	}
#endif
	
private:
	int m_bucket_num;             //number of buckets
	int m_node_num;               //number of node
	int m_free_num;          //number of free node

	bucket_t *m_bucket;       //point to the list of buckets
	block_t  *m_first_block;  //point to the first block
	block_t  *m_cur_block;    //point to the current block
#ifdef _HASH_THREAD_SAFE
	node_t   *m_free_node;    //the list of free node
	pthread_rwlock_t *m_lock;
#endif

	hash_fcn_t m_fcn;          //the hash function
	compare_t m_equal;

#ifdef HASH_MONITOR_X
	int used_bucket;
#endif
};
};
#endif

