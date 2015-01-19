/***************************************************************************
 *
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: ependingpool.h,v 1.9 2009/11/30 08:09:27 baonh Exp $
 *
 **************************************************************************/



/**
 * @file ependingpool.h
 * @author baonh(baonenghui@baidu.com)
 * @version $Revision: 1.9 $
 * @brief ependingpool是一个 TCP 连接池
 *
 *
 *
 * <pre>
 * ependingpool可以对运行中的部分事件设置回调函数，从而实现网络服务模型的不同应用
 * ependingpool的使用参考test下的样例程序
 *
 * </pre>
 **/

#ifndef __EPOLL_PENDING_POOL_
#define __EPOLL_PENDING_POOL_
#include <sys/time.h>


/*
    nsheader网络类

*/


static const unsigned int NSHEAD_MAGICNUM = 0xfb709394;


/// 返回错误码 = NSHEAD_RET_SUCCESS成功, <0失败
typedef enum _NSHEAD_RET_ERROR_T {
    NSHEAD_RET_SUCCESS       =   0, ///<读写OK
    NSHEAD_RET_EPARAM        =  -1, ///<参数有问题
    NSHEAD_RET_EBODYLEN      =  -2, ///<变长数据长度有问题
    NSHEAD_RET_WRITE         =  -3, ///<写的问题
    NSHEAD_RET_READ          =  -4, ///<读消息体失败，具体错误看errno
    NSHEAD_RET_READHEAD      =  -5, ///<读消息头失败, 具体错误看errno
    NSHEAD_RET_WRITEHEAD     =  -6, ///<写消息头失败, 可能是对方将连接关闭了
    NSHEAD_RET_PEARCLOSE     =  -7, ///<对端关闭连接
    NSHEAD_RET_ETIMEDOUT     =  -8, ///<读写超时
    NSHEAD_RET_EMAGICNUM     =  -9, ///<magic_num不匹配
    NSHEAD_RET_UNKNOWN	     =  -10
} NSHEAD_RET_ERROR_T;

/// 读写标志位，所有读写函数默认NSHEAD_CHECK_MAGICNUM
typedef enum _NSHEAD_FLAGS_T {
    NSHEAD_CHECK_NONE        =    0,
    NSHEAD_CHECK_MAGICNUM    = 0x01,    ///<检查magic_num是否正确
    NSHEAD_CHECK_PARAM       = 0x02,    ///<检查参数是否正确
} NSHEAD_FLAGS_T;

/**
* ns产品线网络交互统一的包头，注释包含为(M)的为必须遵循的规范
*/
typedef struct _nshead_t
{
    unsigned short id;              ///<id
    unsigned short version;         ///<版本号
    ///(M)由apache产生的logid，贯穿一次请求的所有网络交互
    unsigned int   log_id;
    ///(M)客户端标识，建议命名方式：产品名-模块名，比如"sp-ui", "mp3-as"
    char           provider[16];
    ///(M)特殊标识，标识一个包的起始
    unsigned int   magic_num;
    unsigned int   reserved;       ///<保留
    ///(M)head后请求数据的总长度
    unsigned int   body_len;
} nshead_t;


//////////////////////////////////////////////////////////////////////////


/**
 * @brief ependingpool类
 *
 * 封装了ependingpool的各种操作
 **/

class ependingpool {
public:
	ependingpool();

	/**
	 * @brief 析构函数， 会把所有未关闭的socket强制关闭
	 *
	**/
	~ependingpool();

	/**
	 * @brief 从就绪队列取出已就绪的socket
	 *
	 * @param [out] *handle   : 用于存储连接句柄
	 * @param [out] *sock     : 用于存储连接socket
	 * @return  0 成功， -1失败
	 * @author baonh
	**/
	int fetch_item(int *handle, int *sock);

	/**
	 * @brief 从就绪队列取出已就绪的socket
	 *
	 * @param [out] *handle   : 用于存储连接句柄
	 * @param [out] *sock     : 用于存储连接socket
	 * @param [out] *staytime     : 句柄在队列中存在的时间(单位: 微秒 us)
	 * @return  0 成功， -1失败,
	 * -1失败可能是调用了stop进行安全退出, 在出错的时候可以再判断一下is_run
	**/
	int fetch_item(int *handle, int *sock, long long *staytime);

	/**
	 * @brief 关闭所有处于就绪状态的socket
	 *
	 * @return  被关闭的sock个数
	**/
	int close_ready_socket();

	/**
	 * @brief 等待ependingpool把剩余的读写事件结束
	 * 这个主要用于在使用了读写事件进行异步读写操作并需要安全退出的时候
	 * 和check_item要在同一个线程中
	 *
	 * @param [in] timeout   : 超时时间,超过时间限制，会强制关闭正在处于写状态的socket
	 * @return  0 成功
	**/
	int waitforstop(int timeout);

    /**
     * @brief 通过handle 获取socket 所带的结构指针(socket由fetch_item获取)
     *
     * @param [in] handle   : int ependingpool句柄
     * @param [in] arg   : void** 返回结果 的指针
	 * @author baonh
    **/
    int fetch_handle_arg(int handle, void **arg);

	/**
	 * @brief 关闭连接句柄
	 *
	 * @param [in] handle   : 连接句柄
	 * @param [in] keep_alive   : false短连接句柄关闭，true长连接句柄不马上关闭
	 * @return  0 成功， -1失败
	 * @author baonh
	**/
	int reset_item(int handle, bool keep_alive);

	/**
	 * @brief 对socket进行异步写操作，会调用由SOCK_WRITE设置的回调函数
	 *
	 * <pre>
	 * 写的数据可以通过由fetch_handle_arg获取的指针进行控制
	 * </pre>
	 *
	 * @param [in] handle   : 连接句柄
	 * @author baonh
	**/
	int write_reset_item(int handle);

	/**
	 * @brief 将handle从ependingpool里移除，但不关闭sock句柄
	 *
	 * @param [in] handle   : 连接句柄
	 * @author baonh
	**/
	int clear_item(int handle);

	/**
	 * @brief 事件发生回调函数的类型
	 *
	 * <pre>
	 * ependingpool中对于每个句柄都对应着一个指针，初始为NULL，由用户进行控制
	 * event_callback_t 第二个参数为void ** 类型，通过对*arg赋值可以传入用户指针
	 * 部分事件对于函数的返回值有相应的要求，请参看说明
	 * </pre>
	 **/
	typedef int event_callback_t(int sock, void **arg);

	struct ependingpool_task_t {
		int sock;		  /**< 使用的sock       */
		int offset;		  /**< sock的偏移       */
		void *arg;		  /**< 与局柄绑定的数据       */
		void *user_arg;	  /**< 用户数据       */
	};

	/**
	 * @brief 设置回调的数据类型
	 *
	 * @param [in] param   : struct ependingpool_task_t
	**/
	typedef int event_callback_ex_t(struct ependingpool_task_t *param);

	/**
	 * @brief ependingpool运行过程中发生的各种事件的定义，可以针对每个事件设置回调函数
	 *
	 * <pre>
	 * 回调函数类型为 @ref event_callback_t
	 *
	 * 参数 sock 表示发生事件的sock
	 * 参数 void **arg 是指向与sock绑定的指针 的指针，通过*arg 进行使用
     *
	 * 各个事件的回调函数说明:
	 *
	 * SOCK_ACCEPT 事件 :
	 * sock表示的是进行监听的端口，需要用户自己进行accept操作
	 * 在没有设置SOCK_ACCEPT情况下只是简单的accept出一个sock
	 * 返回值:
	 * <0 accept失败
	 * >=0 返回建立的sock句柄
	 * 对sock进行setsockopt, 设置非堵塞等操作可以在这里进行
     *
	 * SOCK_INIT sock绑定指针初始化 ,在SOCK_ACCEPT事件触发后
	 * 这个时候可以将与sock绑定的指针传入，并进行初始化
	 * 返回值:
	 * <0 初始化失败，触发SOCK_CLEAR事件，关闭句柄
	 * =0 初始化成功，放入epoll进行监听
     *
	 * SOCK_LISTENTIMEOUT 监听sock超时事件
	 * 当epoll监听的sock超时触发, 回调结束后，触发SOCK_CLEAR事件，关闭sock，
	 * 超时由set_conn_timeo控制
	 * 返回值:
	 * 不判断返回值
	 *
	 *
	 * SOCK_READ 读事件
	 * 这里的读事件是发生在监听到读请求之后发生，需要在sock是非堵塞模式的情况下使用
	 * 用来实现异步读数据
	 * 返回值:
	 * <0 读失败，会触发SOCK_CLEAR事件
	 * =0 数据全部读取完毕，并将sock放入就绪队列中
	 * =1 数据读成功，但没有全部读取完毕，需要再次进行监听，sock会被放回epoll中
	 * =2 数据全部读取完毕，但不将sock放入就绪队列中，而是触发SOCK_TODO事件，可以实现单进程异步处理模式
	 * =3 触发SOCK_CLEAR事件，但不关闭句柄，可以在这里把句柄移出ependingpool
	 *
	 * SOCK_READTIMEOUT sock读超时
	 * 在SOCK_READ 读事件后，发送读超时，回调结束后，触发SOCK_CLEAR事件，关闭sock,
	 * 超时由set_read_timeo控制
	 * 返回值:
	 * 不判断返回值
	 *
     * SOCK_TODO sock处理事件
	 * 当SOCK_READ 事件成功返回 2的时候被触发
	 * 返回值:
	 * <0 操作失败， 触发SOCK_CLEAR事件，关闭句柄
	 * =0 操作成功， 触发SOCK_WRITE 写事件
	 * =1 操作成功， 但不触发SOCK_WRITE 写事件， sock放回epoll中进行监听读请求
	 * =2 操作成功， 触发SOCK_CLEAR事件，但不关闭句柄，可以把句柄移出ependingpool
	 *
     *
	 * SOCK_WRITE 写事件
	 * 与SOCK_READ类似主要用于以异步方式写数据，sock也需要是非堵塞模式
	 * 返回值:
	 * <0 写失败，会触发SOCK_CLEAR事件
	 * =0 数据全部写完毕，并将sock重新放入epoll中进行监听读请求(相当于长连接)
	 * =1 数据写成功，但没有全部写完，放回epoll等待下次再监听到可写状态
	 * =2 数据全部写完毕，关闭sock，会触发SOCK_CLEAR事件(相当于短连接)
	 * =3 触发SOCK_CLEAR事件，但不关闭句柄，可以把句柄移出ependingpool
     *
	 * SOCK_WRITETIMEOUT sock写超时
	 * 在SOCK_WRITE 写事件发生后，写数据超时, 回调结束后，SOCK_CLEAR事件，关闭sock,
	 * 超时由set_write_timeo控制
	 * 返回值:
	 * 不判断返回值
	 *
	 * SOCK_CLEAR sock绑定指针释放
	 * 在需要close句柄前触发，为清理sock绑定指针, 长连接调用reset_item的时候不会触发此事件
     *
	 * 返回值:
	 * 不判断返回值
	 *
	 *
	 * SOCK_FETCH 工作线程fetch到sock触发事件
	 * 在调用fetch_item，获取到可用的sock后触发
	 * 请不要在fetch_item内部调用 reset_item 关闭sock, 通过返回值进行判断
	 * 返回值:
	 * <0 调用sock失败，触发SOCK_CLEAR事件，关闭sock
	 * =0 调用sock成功, 不触发任何事件，外部的工作线程可以使用通过fetch_item获取的句柄
	 * =1 调用sock成功，短连接，触发SOCK_CLEAR事件，关闭sock
	 * =2 调用sock成功，长连接，不触发sock关闭事件，sock放回epoll中继续监听
	 * =3 调用sock成功，触发SOCK_CLEAR事件，将sock清出sock池，但不关闭sock句柄，可以放入另外的pool中
	 *
	 * 注意，调用fetch_item的返回值与SOCK_FETCH事件的返回值相同
	 * 出现非0的返回值时，fetch出来的sock是不可用的
	 * 因为这个事件发生在工作线程中，请注意线程安全问题
	 *
	 * SOCK_QUEUEFAIL sock放入队列失败触发
	 * 返回值:
	 * =0 触发SOCK_CLEAR事件，关闭sock句柄
	 * =1 触发SOCK_CLEAR事件，但不关闭sock句柄，将句柄移出ependingpool
	 *
	 * SOCK_INSERTFAIL sock加入sock池失败时触发
	 * 返回值:
	 * 不判断返回值
	 *
	 * 注意: SOCK_INSERTFAIL 事件触发在SOCK_INIT事件之前
	 *
	 *
	 *
	 * </pre>
	 **/
	typedef enum {
	    SOCK_ACCEPT = 0,          /**< 建立连接事件       */
		SOCK_INIT,				  /**< sock绑定指针初始化事件      */
		SOCK_LISTENTIMEOUT,		  /**< 监听  sock超时事件      */
		SOCK_READ,		          /**< 读事件      */
		SOCK_READTIMEOUT,		  /**< sock读数据超时事件      */
		SOCK_TODO,		          /**< sock处理事件       */
		SOCK_WRITE,		          /**< 写事件       */
		SOCK_WRITETIMEOUT,		  /**< sock写数据超时事件      */
		SOCK_CLEAR,		          /**< sock绑定指针释放事件      */
		SOCK_FETCH,		          /**< 工作线程fetch到sock触发事件       */
		SOCK_QUEUEFAIL,		      /**< 句柄放入队列失败时触发       */
		SOCK_ERROR,
		SOCK_HUP,
		SOCK_CLOSE,
		SOCK_INSERTFAIL,		  /**< 给ependingpool插入sock失败时触发       */
		SOCK_EVENT_NUM		      /**< 事件的总个数       */
	} ependingpool_event_type;


	/**
	 * @brief 对ependingpool对运行过程中发生的事件 设置回调函数
	 *
	 * 当相应的事件被触发的时候会运行回调函数
	 *
	 *
	 * 这个回调函数主要考虑到在accept出新的sock后需要进行setsockopt操作
	 *
	 * @return  0 成功， -1失败
	 * @author baonh
	 **/

	int set_event_callback(int event, event_callback_t *callback);

	/**
	 * @brief TODO事件回调,可以获取offset
	 *
	 * @param [in] callback   :  参见event_callback_ex_t
	 * @param [in] user_arg   :  用户数据, 存放在在event_callback_ex_t的user_arg中
	 *
	 * @return
	 * * <0 操作失败， 触发SOCK_CLEAR事件，关闭句柄
	 * =0 操作成功， 触发SOCK_WRITE 写事件
	 * =1 操作成功， 但不触发SOCK_WRITE 写事件， sock放回epoll中进行监听读请求
	 * =2 操作成功， 触发SOCK_CLEAR事件，但不关闭句柄，可以把句柄移出ependingpool
	 * =3 什么事情也不做(相当于feth_item后的状态,可以通过在callback获取的数据进行操作,
	 *	  需要调用reset_item对socket进行操作
	**/
	int set_todo_event_ex(event_callback_ex_t *callback, void *user_arg);
	/**
	 * @brief 设置监听socket
	 *
	 * @param [in] lis_fd   : 监听socket
	 * @return  0成功 -1失败
	 * @author baonh
	 **/
	int set_listen_fd(int lis_fd);

	/**
	 * @brief 设置已就绪队列的长度(默认:300)
	 *
	 * @param [in] len   : 已就绪队列的长度
	 * @return 0 成功 -1 失
	 * @note 必须在线程访问pool前设置 , 不要在运行中修改
	 * @author baonh
	**/
	int set_queue_len(int len);

	/**
	 * @brief 设置可存储socket的数量(默认:500)
	 *
	 * @param [in/out] num   : 可存储socket的数量
	 * @return  0 成功 -1 失败
	 * @note 必须在线程访问pool前设置 , 不要在运行中修改
	 * @author baonh
	**/
	int set_sock_num(int num);

	/**
	 * @brief 设置连接超时(单位:秒, 默认:1s)
	 *
	 * @param [in] timeo   : 超时时间
	 * @return  0 成功 -1 失败
	 * @note  这里的连接超时不包括fetch_item以后的时间
	 * @author baonh
	**/
	int set_conn_timeo(int timeo);

	/**
	 * @brief 对于使用了SOCK_READ的情况, 设置读超时(单位:秒, 默认:1s)
	 *
	 * @param [in] timeo   : 超时时间
	 * @return  0 成功 -1 失败
	 * @author baonh
	**/
	int set_read_timeo(int timeo);

	/**
	 * @brief 对于设置了SOCK_WRITE的情况,设置写超时(单位:秒, 默认:1s)
	 *
	 * @param [in] timeo   : 超时时间
	 * @return  0 成功 -1 失败
	 * @author baonh
	**/
	int set_write_timeo(int timeo);



	/**
	 * @brief 设置epoll每次wait时间(单位:ms,默认10ms)
	 *
	 * @param [in] timeo : epoll 等待时间
	 * @return  0 成功 -1 失败
	 * @note  这个设置需要比set_conn_timeo 的时间要小，否则会出现set_conn_timeo关闭句柄不及时的情况
	 * @author baonh
	**/
	int set_epoll_timeo(int timeo);

	/**
	 * @brief 获取当前队列的长度
	 *
	 * @return  队列的长度
	 * @author baonh
	**/
	int get_queue_len();

	/**
	 * @brief 获取当前连接超时时间(秒)
	 *
	 * @return  获取当前连接超时时间
	 * @author baonh
	**/
	int get_conn_timeo();

	/**
	 * @brief 获取当前读超时时间(秒)
	 *
	 * @return  获取当前写超时时间
	 * @author baonh
	**/
	int get_read_timeo();

	/**
	 * @brief 获取当前写超时时间(秒)
	 *
	 * @return  获取当前写超时时间
	 * @author baonh
	**/
	int get_write_timeo();


	/**
	 * @brief 获取epoll　wait 的时间(ms)
	 *
	 * @return  epoll　wait 的时间
	 * @author baonh
	**/
	int get_epoll_timeo();
	/**
	 * @brief 获取空闲的线程数
	 *
	 * @return  空闲的线程数
	 * @author baonh
	**/
	int get_free_thread();

	enum { DEF_MAX_SOCK = 500 };		  /**< 默认存储socket的数量       */
	enum { DEF_QUEUE_LEN = 100 };		  /**< 默认就绪队列长度       */
	enum { DEF_CONN_TIMEO = 2 };		  /**< 默认连接超时时间       */
	enum { DEF_READ_TIMEO = 1 };		  /**< 默认读超时       */
	enum { DEF_WRITE_TIMEO = 1 };   	  /**< 默认写超时       */
	enum { DEF_EPOLL_TIMEO = 10 };		  /**< 默认epoll超时时间     */

	typedef enum {
		NOT_USED = 0,		  /**< socket处于未使用状态       */
		READY,		          /**< socket处于就绪状态，等待请求       */
		READ_BUSY,		      /**< socket处于读状态       */
		BUSY,		          /**< socket处于忙碌状态, 在队列中或者被线程fetch到       */
		WRITE_BUSY		      /**< socket处于写状态       */
	} sock_status_t;

	/**
	 * @brief 检查各socket的状态
	 *
	 * 获取新建建立的socket
	 * 将为被epool监听的线程放入epoll
	 * 检查超时的socket, socket超时会被关闭
	 *
	 * @return  0 检查成功　-1 失败
	 * @author baonh
	**/
	int check_item();

	/**
	 * @brief 重新设置ependingpool为可运行状态
	 *
	 * @author baonh
	**/
	int restart_run();

	/**
	 * @brief ependingpool进行安全退出,可以结合is_run()判断ependingpool的退出
	 *
	 * 使用前先确定工作线程已经处于停止状态，可以通过判断get_free_thread
	 *
	 * @return  int
	 * @author baonh
	**/
	int stop();

	/**
	 * @brief 判断ependingpool是否处于运行状态, 主要用于进程的安全退出，停止可以由stop控制
	 *
	 * @return  0 ependingpool处于关闭状态 1 ependingpool处于可运行状态
	 * @author baonh
	**/
	int is_run();

	/**
	 * @brief 将sock直接放入ependingpool进行监听
	 *
	 * @param [in] sock   : 需要进行监听的局柄
	 * @param [in] arg    : 与sock捆绑的指针，在为NULL时会调用SOCK_INIT回调,
	 *                      否则不会触发SOCK_INIT事件, 但句柄关闭时还是会触发SOCK_CLEAR事件
	 *                      使用时请注意
	 * @param [in] flags  : 0表示监听读操作，非0 表示监听写操作, 默认为监听读操作
	 * @return >0 为sock在ependingpool中的句柄handle, < 0为失败
	 * @note 失败的时候不会主动关闭sock, 请在外部将sock关闭， arg 也不会调用SOCK_CLEAR事件
	 *       请注意释放内存.
	 *       默认情况下接口是线程不安全，需要使用set_insert_item_sock_thrsafe进行设置。使用时请注意
     *       2013-02-01修改 feimat@baidu.com
     *       为适应异步模块，增加返回offset的位置，以便可以操作 write_reset_item

	**/
	int insert_item_sock(int sock,int &async_offset, void *arg = NULL, int flags = 0);

	/**
	 * @brief
	 *
	 * insert_item_sock默认情况下是线程不安全的，set_insert_item_sock_thrsafe(1)
	 * 可以使insert_item_sock线程安全
	 *
	 * @param [in] thrsafe   : 非０insert_item_sock内部会加锁线程安全，０内部不加锁，线程不安全
	 * @return　0设置成功，-1设置失败
	 * @author baonh
	**/
	int set_insert_item_sock_thrsafe(int thrsafe);
private:

	/**
	 * @brief 检查超时的情况
	 *
	 * @return  int
	 * @author baonh
	**/
	int check_time_out();

	/**
	 * @brief 获取空闲的位置用于存储新socket
	 *
	 * @return  空闲的位置句柄
	 * @author baonh
	**/
	int get_offset();
	/**
	 * @brief 将socket放入
	 *
	 * @param [in] offset   :　空闲的位置句柄，　由get_offset获取
	 * @return  0 放入成功，-1放入失败
	 * @note	放入失败的同时会把socket关闭
	 * @author baonh
	**/
	int insert_item(int offset);
	/**
	 * @brief 将已经准备就绪的socket放入就绪队列中
	 *
	 * @param [in] offset   : 已就绪的socket的offset句柄
	 * @return  0 放入成功，-1放入失败
	 * @note	放入队列失败，不会马上close，会在下次再次进行检测
	 * @author baonh
	**/
	int queue_in(int offset);

	/**
	 * @brief 与队列操作相关的函数
	 *
	 * @param [in/out] queue_size   : int
	 * @note
	 * @author baonh
	**/
	int queue_create(int queue_size);
	int queue_push(int val);
	int queue_pop(int* val);
	int queue_empty();
	int queue_full();
	int queue_destroy();
	/**
	 * @brief accept 由set_listen_fd设置的句柄
	 *
	 * @return  新的连接句柄
	 * @author baonh
	**/
	int accept_sock();

	/**
	 * @brief epoll等待
	 *
	 * @param [in] timeout   : 等待时间
	 * @author baonh
	**/
	int pool_epoll_wait(int timeout);

	int pool_epoll_add(int socket, int fd, int event);
	int pool_epoll_del(int socket, int fd);
	int pool_epoll_mod(int socket, int fd, int event);

	int pool_epoll_offset_add(int offset, int event);
	int pool_epoll_offset_del(int offset);
	int pool_epoll_offset_mod(int offset, int event);

	void do_read_event(int offset);
	void do_write_event(int offset);
	void do_epollhup(int offset);
	void do_epollerr(int offset);

	typedef struct _sock_item {
		int sock_status;			  /**< socket状态       */
		int sock;					  /**<　存储的socket       */
		int last_active;		  /**< 最后活动的时间,判断超时使用       */
		void *arg;
		struct timeval queue_time;		  /**< socket放入队列的时间       */
	} sock_item_t;

	int m_sock_num;		          /**< 存储的socket数目       */
	sock_item_t *m_ay_sock;		  /**< 用于存放socket的空间     */

	int m_sock_len;		          /**< 实际存储的socket数目       */
	int m_least_last_update;	  /**<所有处于ready状态socket 最后活动时间　最小的时间    */

	int m_queue_len;			  /**< 队列最大长度       */
	int *m_ay_ready;			  /**< 就绪队列       */
	int m_get;					  /**< 队列头       */
	int m_put;					  /**< 队列尾       */

	struct epoll_event *m_ay_events;		  /**< 存储epoll的事件队列       */

	int m_free_thr;				  /**< 获取等待获取连接的线程数       */
	int m_conn_timeo;			  /**< 连接超时时间        */
	int m_read_timeo;		  /**< 读超时       */
	int m_write_timeo;		  /**< 写超时      */
	int m_min_timeo;
	int m_epfd;					  /**< epoll 句柄       */
	int m_ep_timeo;				  /**< epoll 等待时间       */
	int m_insert_item_sock_thrsafe;

	int m_listen_fd;			  /**< 监听句柄       */

	event_callback_t *m_callbacklist[SOCK_EVENT_NUM];

	void *m_todo_event_user_arg;
	event_callback_ex_t *m_todo_event_callback;

	int pool_run;


	int m_read_socket_num;
	int m_write_socket_num;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_condition;
	pthread_mutex_t m_insert_sock;
};

#endif //__EPOLL_PENDING_POOL_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
