/**
 * @file
 * @brief 字符及字符串处理
 */
#ifndef _UL_MODULE_STRING_
#define _UL_MODULE_STRING_

extern int UL_CHAR_SPACE[];
extern char legal_char_set[];
extern char url_eng_set[];
extern char legal_word_set[];

/**
 *  判断给定的字符是否为空格\n
 *  字符串存放到dststr中，srcstr保持不变
 *  
 *  @param[in]  ch 待检测的字符
 *  @return 返回判断结果
 *  - 0 表示不是空格
 *  - 1 表示是空格
 *  @note 注意space字符包括'\0',这与函数isspace不同
 */
#define ul_isspace(ch) UL_CHAR_SPACE[(unsigned char)(ch)]

/**
 *  将字符串中的大写字母转换为小写字母
 *  
 *  @param[in]  pstr 待转换的字符串
 *  @param[out] pstr 转换后的字符串
 *  @return 返回处理结果
 *  - 0 表示转换失败
 *  - 1 表示转换成功
 *  @note 如果用来转换gbk字符串,gbk单词可能被修改,慎用,建议中文改用ul_trans2lower
 */
int ul_tolowerstr_singlebuf(unsigned char *pstr);

/**
 *  将大写字母转换为小写字母，非大写字母则返回字母本身
 *  
 *  @param[in]  srcchar 待转换的字符
 *  @param[out] 无
 *  @return 返回转换后的字符
 */
int ul_tolower(unsigned char srcchar);

/**
 *  将字符串中的大写字母转换为小写，并将转换后的\n
 *  字符串存放到dststr中，srcstr保持不变
 *  
 *  @param[in]  srcstr 待转换的字符串
 *  @param[in]  dststr 存放转换后的字符串的缓冲区
 *  @param[out] dststr 转换后的字符串
 *  @return 返回处理结果
 *  - 0 表示转换失败
 *  - 1 表示转换成功
 *  @note 注意dststr的大小>=strlen(srcstr)+1
 */
int ul_tolowerstr(unsigned char *dststr, unsigned char *srcstr);

/**
 *  从字符串中取出第一个单词，并返回下一个单词的位置
 *  
 *  @param[in]  ps_src    源字符串
 *  @param[in]  ps_result 得到的单词
 *  @param[in]  charset   转换使用的矩阵
 *  @param[out] 无
 *  @return 下一个单词的位置
 */
char *ul_sgetw(char *ps_src, char *ps_result, char *charset);

/**
 *  从字符串中取出第一个单词，并返回下一个单词的位置，支持双字节编码
 *  
 *  @param[in]  ps_src    源字符串
 *  @param[in]  ps_result 得到的单词
 *  @param[in]  charset   转换使用的矩阵
 *  @param[out] 无
 *  @return 下一个字符串的位置
 */
char *ul_sgetw_ana(char *ps_src, char *ps_result, char *charset);


/**
 *  不区分大小写字符串比较(利用map实现)
 *  
 *  @param[in]  s1 字符串1 
 *  @param[in]  s2 字符串2
 *  @param[in]  n  待比较长度
 *  @param[out] 无
 *  @return 是否相等
 * - 1 相等
 * - 0 不相等
 */
int ul_strncasecmp(const char *s1, const char *s2, size_t n);

/**
 * @brief 	The strlcpy() function copies up to size - 1 characters 
 * 			from the NUL-terminated string src to dst, NULL-terminating the result.
 *
 * @param [out] dst   : char*	目标缓冲区
 * @param [int] src   : const char*	src 源缓冲区
 * @param [in] siz   : size_t 目标缓冲区的大小
 * @return  size_t 返回应该期望的字符串长度，注意可能会超过缓冲区大小
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/03 21:20:35
**/
size_t ul_strlcpy(char *dst, const char *src, size_t siz);

/**
 * @brief  	appends the NUL-terminated string src to the end of dst.  
 * 			It will append at most size - strlen(dst) - 1 bytes, NUL-terminating the result.
 *
 * @param [out] dst   : char*	destination buffer
 * @param [in] src   : const char*		
 * @param [in] siz   : size_t	
 * @return  size_t 返回应该期望的字符串长度，注意可能会超过缓冲区大小
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2007/12/03 21:28:37
**/
size_t ul_strlcat(char *dst, const char *src, size_t siz);

#endif
