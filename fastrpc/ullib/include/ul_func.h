/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: ul_func.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_func.h
 * @author baonh(baonh@baidu.com)
 * @date 2007/12/07 14:50:45
 * @version $Revision: 1.2 $ 
 * @brief 基本函数库
 *
 * 该函数库中的函数是对libc库函数的包装，增加了一定的参数合法性检查和错误日志打印功能
 *  
 **/

#ifndef __UL_FUNC_H_
#define __UL_FUNC_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>


//**************************
// function declare
//**************************

////////////////////////////////////////////////////////////////////////////////
//         next functions were wraped function about memory fatch             //
////////////////////////////////////////////////////////////////////////////////

//*********************************************************************
// allocates memory for an array of number elements of size bytes each and 
// return s a pointer to the allocated memory.  The memory is set to zero.
//  wrap include the LOG function.
//**********************************************************************
void  * ul_calloc(size_t n, size_t size);

/**
 * @brief 内存分配并清０
 *
 * malloc内，并用memset将内存清０，有时会影响效率
 * 
 * @param [in] size   : size_t
**/
void *ul_malloc(size_t size);

//*********************************************
// realloc() returns a pointer to the newly allocated memory, which is suitably // aligned for any kind  of variable and may be different from ptr,or NULL if 
// the request fails or if size was equal to 0.
//*********************************************
void *ul_realloc(void *ptr, size_t size);

//*********************************************
// frees the memory space pointed to by ptr, which must have been returned by a 
// previous call to ul_Malloc(), ul_Calloc() or ul_Realloc().
// wrap include the LOG function and parameter check
//*********************************************
void ul_free(void *ptr);

////////////////////////////////////////////////////////////////////////////////
//         next functions were wraped function about I/O call                 //
////////////////////////////////////////////////////////////////////////////////

//***************************************************
// wrap function about system call open
//***************************************************
int ul_open(const char *pathname, int oflag, mode_t mode);

//************************************************
// wrap function about system call read
// This function return the data size which it read really.
//************************************************
ssize_t ul_read(int fd, void *ptr, size_t nbytes);

//************************************************
// wrap function about system call write
// This function return the data size which it write really.
//************************************************
ssize_t ul_write(int fd, void *ptr, size_t nbytes);

//************************************************
// wrap function about system call close
//************************************************
int ul_close(int fd);

//************************************************
// wrap function about system call fcntl
// If success,return 0,else return -1
//************************************************
int ul_fcntl(int fd, int cmd, int arg);

//*****************************************************
// ul_dup() create a copy of the file descriptor oldfd.
// It uses the lowest-numbered unused descriptor for the new descriptor.
// Return: return the new descriptor if success.otherwize return -1
//*****************************************************
int ul_dup(int oldfd);

//*****************************************************
// ul_dup2() create a copy of the file descriptor oldfd.
// makes newfd be the copy of oldfd, closing newfd first if necessary.
// Return:return the new descriptor if success.otherwize return -1
//*****************************************************
int ul_dup2(int oldfd, int newfd);

//**************************************************
// ul_lseek() function repositions the offset of the file descriptor fildes to 
// the argument off璼et according to the directive whence as follows:
//     			SEEK_SET(from 0 position);
//				SEEK_CUR(from current position);
//				SEEK_END(from tail)
// Return: if success,it returns the esulting offset.otherwize returns -1
//**************************************************
off_t ul_lseek(int fildes, off_t offset, int whence);

//**************************************************************
// ul_Pipe  creates  a  pair of file descriptors, pointing to a pipe inode, and
// places them in the array pointed to by filedes.  filedes[0] is for reading,
// filedes[1] is for writing.
// Return: 0 success, -1 failed.
//**************************************************************
int ul_pipe(int fds[2]);

//*******************************************************************
// ul_stat() return information about the specified file named by file name.
// The stat of a file was be set in a data struct <struct stat>
// Return: 0 means success,and -1 meads failed.
//*******************************************************************
int ul_stat(const char *file_name, struct stat *buf);

//*******************************************************************
// ul_fstat() return information about the specified file named by file descripter .
// The stat of a file was be set in a data struct <struct stat>
// Return: 0 means success,and -1 meads failed.
//*******************************************************************
int ul_fstat(int filedes, struct stat *buf);

//**************************************************************
// ul_unlink() will delete on file.But if any  processes
//      still have the file open the file will remain in existence
// Return: if success, 0 returns;otherwize -1 returned.
//**************************************************************
int ul_unlink(const char *pathname);

//***********************************************
// wrap function about call fchmod
//***********************************************
int ul_fchmod(int fildes, mode_t mode);

//***********************************************
// wrap function about call fchown
//***********************************************
int ul_fchown(int fildes, uid_t owner, gid_t group);

//***********************************************
// wrap function about call chdir
//***********************************************
int ul_chdir(const char *path);

//***********************************************
// wrap function about call fchdir
//***********************************************
int ul_fchdir(int fildes);

//******************************************************************
// Truncate causes the file named by path to be truncated to at most length bytes in size.
// Return: 0 -- success, -1 -- failed.
//******************************************************************
int ul_truncate(const char *path, off_t length);

//******************************************************************
// ul_ftruncate causes the file named by referenced by fd to be truncated to at most length bytes in size.
// Return: 0 -- success, -1 -- failed.
//******************************************************************
int ul_ftruncate(int fd, off_t length);

//*****************************************************************************
// Wrap function about fpathconf().This function gets  a  value  for  the configuration option name for the
// open file descriptor filedes.
// Note:on failed,return -1.If the system does not have a limit,-100 is returned.
//*****************************************************************************
long ul_fpathconf(int filedes, int name);

//***********************************************************************
// Wrap function about pathconf().
// Note:on failed,return -1.If the system does not have a limit,-100 is returned.
//***********************************************************************
long ul_pathconf(char *path,int name);

////////////////////////////////////////////////////////////////////////////////
//         next functions were wraped function about mmap file           //
////////////////////////////////////////////////////////////////////////////////

#ifdef _POSIX_MAPPED_FILES
//**********************************************************************
// ul_Mmap() function asks to map length bytes starting at offset from the file 
// (or other object) specified by fd into memory, preferably at address start.
// On success,returns a pointer to the mapped  area.On error,returns -1
// Note: You can execute 'man mmap' to now how to use this command
//**********************************************************************
char *ul_mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset);

//*************************************************************************
// wrap about function munmap
// Note: here,the parameter 'start' was the point which returned by call mmap()
// on success, munmap returns 0, on failure -1
//*************************************************************************
int ul_munmap(void *start, size_t length);
#endif

////////////////////////////////////////////////////////////////////////////////
//             next functions were wraped functiorn about stdio               //
////////////////////////////////////////////////////////////////////////////////

//************************************************
// wrap function about fopen()
// Return: success,a FILE point will be returned.Otherwise,NULL was returned.
//************************************************
FILE * ul_fopen(const char *filename, const char *mode);

//************************************************
// wrap function about fclose()
// Return: success,0 was returned.Otherwise,-1 was returned.
//************************************************
int ul_fclose(FILE *fp);

//*******************************************************************
// wrap function about call fdopen().
// fdopen() associates a stream with the existing file descriptor
//*******************************************************************
FILE * ul_fdopen(int fd, const char *mode);

//**************************************************
// Wrap function about freopen()
//**************************************************
FILE *ul_freopen (const char *path, const char *mode, FILE *stream);

//*******************************************************************
// wrap function about call fgets().
// on error,returns NULL
//*******************************************************************
char *ul_fgets(char *ptr, int n, FILE *stream);

//*******************************************************************
// wrap function about call fputs().
// return a non - negative number on success, or EOF on error
//*******************************************************************
int ul_fputs(const char *ptr, FILE *stream);

//*******************************
// Wrap function about fseek
//*******************************
int ul_fseek( FILE *stream, long offset, int whence);

//*******************************
// Wrap function about ftell
//*******************************
long ul_ftell(FILE *stream);

//*******************************
// Wrap function about rewind
//*******************************
void ul_rewind( FILE *stream);

//*******************************
// Wrap function about fgetpos
//*******************************
int ul_fgetpos( FILE *stream, fpos_t *pos);

//*******************************
// Wrap function about fsetpos
//*******************************
int ul_fsetpos( FILE *stream, fpos_t *pos);

//****************************************************************
// Wrap fuction about remove()
//****************************************************************
int ul_remove(const char *pathname);

//*****************************************************************
// Wrap function about rename()
//*****************************************************************
int ul_rename(const char *oldpath, const char *newpath);

//*******************************************************
// Wrap function about popen()
//*******************************************************
FILE *ul_popen(const char *command, const char *type);

//*******************************************************
// Wrap function about pclose()
//*******************************************************                  
int ul_pclose(FILE *stream);

/////////////////////////////////////////////////////////////////////////////
//                           directory functions                           //
/////////////////////////////////////////////////////////////////////////////

//*******************************************
// Wrap function about opendir()
// on error,return NULL
//*******************************************
DIR *ul_opendir(const char *path);

//******************************************************
// Wrap function about readdir()
// on error, NULL was returned.
//******************************************************
struct dirent *ul_readdir(DIR *dir);

//****************************************************
// Wrap function about closedir()
// on success,0 was returned.Otherwise, -1 returned.
//****************************************************
int ul_closedir(DIR *dirp);

/////////////////////////////////////////////////////////////////////////////////////////
//                               Signal fucntion                                       //
/////////////////////////////////////////////////////////////////////////////////////////

//******************************************************
// Wrap function about kill()
// On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
//******************************************************
int ul_kill(pid_t pid, int sig);

//***************************************************************
// wrap function about raise()
//***************************************************************
int ul_raise(int sig);

//******************************************************
// Wrap function about alarm()
//******************************************************
unsigned int ul_alarm(unsigned int secs);

//******************************************************
// Wrap function about sigemptyset()
// return 0 on success and -1 on error.
//******************************************************
int ul_sigemptyset(sigset_t *set);

//******************************************************
// Wrap function about sigfillset()
// return 0 on success and -1 on error.
//******************************************************
int ul_sigfillset(sigset_t *set);

//******************************************************
// Wrap fucntion aboout sigaddset()
// return 0 on success and -1 on error.
//******************************************************
int ul_sigaddset(sigset_t *set, int signum);

//******************************************************
// wrap function about sigdelset()
// return 0 on success and -1 on error.
//******************************************************
int ul_sigdelset(sigset_t *set, int signum);

//******************************************************
// Wrap function about sigismember()
// returns 1 if signum is a member of set,0 if signum is not a member,and -1 on error.
//******************************************************
int ul_sigismember(const sigset_t *set, int signum);

//******************************************************************
// Wrap function about sigaction()
// return 0 on success and -1 on error.
//******************************************************************
int ul_sigaction(int signo,const struct sigaction *act,struct sigaction *oact);

//******************************************************************
// Wrap function about sigprocmask()
// return 0 on success and -1 on error.
//******************************************************************
int ul_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

//******************************************************************
// Wrap function about sigpending()
// return 0 on success and -1 on error.
//******************************************************************
int ul_sigpending(sigset_t *set);

//******************************************************************
// Wrap function about sigsuspend()
// return 0 on success and -1 on error.
//******************************************************************
int ul_sigsuspend(const sigset_t *mask);

//*************************************************************
// Wrap function about pause()
// pause always returns -1, and errno is set to ERESTARTNOHAND.
//*************************************************************
int ul_pause(void);

////////////////////////////////////////////////////////////////////////////////////
//                          Create process function                              //
////////////////////////////////////////////////////////////////////////////////////

//************************************************
// Wrap function about fork()
//************************************************
pid_t ul_fork(void);

//************************************************
// Wrap function about exec
//************************************************
int ul_execve(const char *filename, char *const argv [], char *const envp[]);

//**************************************
//  Wrap function about wait()
//**************************************
pid_t ul_wait(int *status);

//**************************************
//  Wrap function about waitpid()
//**************************************
pid_t ul_waitpid(pid_t pid, int *status, int options);

//*********************************************
// Wrap function about getpid()
//*********************************************
pid_t   ul_getpid();

//*********************************************
// Wrap function about setpgid()
//*********************************************
int ul_setpgid(pid_t pid, pid_t pgid);

//*********************************************
// Wrap function about getpgid()
//*********************************************
pid_t ul_getpgid(pid_t pid);

//*********************************************
// Wrap function about setpgrp()
//*********************************************
int ul_setpgrp(void);

//*********************************************
// Wrap function about getpgrp()
//*********************************************
pid_t   ul_getpgrp();

/////////////////////////////////////////////////////////////////////////////////////////////
//                                        IPC functions                                 //
/////////////////////////////////////////////////////////////////////////////////////////////

//*******************************************************************
//  Wrap function about mkfifo()
//*******************************************************************
int ul_mkfifo ( const char *pathname, mode_t mode );

//*******************************************************************
// Wrap function about ftok()
//*******************************************************************
key_t ul_ftok( char *pathname, char proj );

//*******************************************************************
// Wrap function about shmget()
//*******************************************************************
int ul_shmget(key_t key, size_t size, int shmflg);

//*******************************************************************
// Wrap function about shmat()
// Return value: return -1 if failed.
//*******************************************************************
void *ul_shmat ( int shmid, const void *shmaddr, int shmflg );

//*******************************************************************
//  Wrap function about shmdt()
//*******************************************************************
int ul_shmdt ( const void *shmaddr);

//*******************************************************************
//  Wrap function about shmctl()
//*******************************************************************
int ul_shmctl(int shmid, int cmd, struct shmid_ds *buf);

//*******************************************************************
//  Wrap function about semget()
//*******************************************************************
int ul_semget( key_t key, int nsems, int semflg );

//*******************************************************************
//  Wrap function about semop()
//*******************************************************************
int ul_semop( int semid, struct sembuf *sops, unsigned nsops );

#ifndef SEMUN_DEFINED
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
	union semun {
		int val;                    /* value for SETVAL */
		struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
		unsigned short int *array;  /* array for GETALL, SETALL */
		struct seminfo *__buf;      /* buffer for IPC_INFO */
	};
#define SEMUN_DEFINED        1
#endif
#endif

//*******************************************************************
//  Wrap function about semctl()
//*******************************************************************                      
int ul_semctl (int semid, int semnum, int cmd, union semun arg);

//*******************************************************************
//  Wrap function about msgget()
//*******************************************************************
int ul_msgget(key_t key, int msgflg );

//*******************************************************************
//  Wrap function about msgctl()
//*******************************************************************
int ul_msgctl(int msqid, int  cmd, struct msqid_ds *buf );

//*******************************************************************
//  Wrap function about msgsnd()
//*******************************************************************
int ul_msgsnd (int msqid, struct msgbuf *msgp, size_t msgsz, int msgflg);

//*******************************************************************
//  Wrap function about msgrcv()
//*******************************************************************
ssize_t ul_msgrcv (int msqid, struct msgbuf *msgp, size_t msgsz, long msgtyp, int msgflg);

#endif // __UL_FUNC_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
