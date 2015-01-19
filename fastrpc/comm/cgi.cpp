//For GET method : max name and val len is CGI_PARA_LEN(256)

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "cgi.h"


CGI::CGI()
{
	var_count=0;
	has_checkbox=0;
	cookie=getenv("HTTP_COOKIE");
	upload_filename=0;
	m_szBuffer = (char *)malloc(1024*20);
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	
	PrintHTTPHead();
	remote_user=0;
	GetInput();
}

CGI::CGI(char *pData)
{
	var_count=0;
	has_checkbox=0;
	cookie=getenv("HTTP_COOKIE");
	upload_filename=0;
	m_szBuffer = (char *)malloc(1024*20);
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	
	remote_user=0;
	GetInput();
}

CGI::~CGI()
{
	free(m_szBuffer);
}

CGI::CGI::CGI(int i)
{
	var_count=0;
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	has_checkbox=0;
	cookie=getenv("HTTP_COOKIE");
	upload_filename=0;
	if (i)
		PrintHTTPHead();
}

char* CGI::operator[](const char *name)
{
	/*if((method!=1)&&(method!=2)) 
	{
		cout<<"Enter var "<<name<<"'s value:";
		char *val=new char[100];
		if(!val) 
		{
			cout<<"Out of Memory!"<<endl; 
			exit(0);
		}
		cin>>val;
		if(strlen(val)&&(val[0]!='.')) 
			return val;
		else 
			return NULL;
	}*/
	return FindValue(name);
}

void CGI::PrintHTTPHead()
{
	printf("Content-Type:text/xml; charset=utf-8\n"); 
//	printf("Server: Sina chatroom\n");
	printf("Cache-Control: no-cache\n");
	//printf("Accept-Ranges: Bytes\n");
	//printf("Connection: close\n");
	printf("\n");
}

void CGI::PrintHTTPHead(int nMsgLength)
{
	
	printf("Content-Type:text/xml; charset=utf-8\n"); 
	if (nMsgLength > 0)
	{
		printf("Content-Length: %d\n", nMsgLength);
	}
//	printf("Server: Sina chatroom\n");
	printf("Cache-Control: no-cache\n");
	//printf("Accept-Ranges: Bytes\n");
	//printf("Connection: close\n");
	
	printf("\n");
}

#define CGI_PARA_LEN	256	
const char multipart[]="multipart/form-data";

void CGI::GetInput()
{
	char *p;
	int i,j,k;
	int content_len = 0;
	
	i=0;
	GetMethod();
	if(method==2)  //method:POST
	{
			p=getenv("CONTENT_LENGTH");
			if(p== NULL) 
			{
				var_count= 0; 
				content_len=0;
			}
			else 
				content_len= atoi(p);
			
			if(content_len >=1024*20)
			{
				//WriteDirLog((char *)"/home/sms/wangyang/provision/smsapachemode/src/log/test.log",(char *)"post data so long:%d", content_len);
				exit(0);
			}
			//memset(m_szBuffer, 0, sizeof(m_szBuffer));
			fread(m_szBuffer,1, content_len, stdin);
			//WriteDirLog((char *)"/home/sms/wangyang/provision/smsapachemode/src/log/test.log",(char *)"neirong[%s][%d]",m_szBuffer+4,content_len);
			var_count = content_len;
	}
	else if(method==1)  //method:GET
	{
		p=getenv("QUERY_STRING");
		if(!p) 
		{
			var_count=0; 
			content_len=0;
		}
		else 
		{
			content_len=strlen(p);
			k=0;
			for(i=0;k<content_len;i++)
			{
				input_data[i].val=new char[CGI_PARA_LEN+1];
				j=0;
				while(p[k]&&(p[k]!='&')&&(j<CGI_PARA_LEN))
					(input_data[i].val)[j++]=p[k++]; 
				(input_data[i].val)[j]='\0';
				while(p[k]&&(p[k]!='&')) k++;   
				if(p[k]=='&') k++;
				AddToSpace(input_data[i].val);
				Convert(input_data[i].val);
				input_data[i].name = ReadData(input_data[i].val,'=');
			}
			var_count = i;
		}
	}
}

char *CGI::GetData()
{
	//if(m_szBuffer[0] == 0)
	//	return NULL;
	return m_szBuffer;
}

void CGI::GetMethod()
{
	char *meth;
	meth=getenv("REQUEST_METHOD");
	if(!meth) method=0;
	else if(!strcmp(meth,"POST"))
		method=2;	//post=2
	else if(!strcmp(meth,"GET")) 
		method=1;	//get=1
	else method=3;
}

int CGI::GetRemoteUser()
{
	remote_user=getenv("REMOTE_USER");
	if(remote_user) 
		return 1;
	else 
		return 0;
}

char* CGI::FindValue(const char *s)
{
	int i;
	int bfound=0;
	char *p=NULL;
	int buf_len=0;
	for(i = 0;i<var_count;i++)
	{
		if(!strcasecmp(s,input_data[i].name))
		{ 
			if(has_checkbox==0)
			{
				if(input_data[i].val&&input_data[i].val[0])
					return input_data[i].val;
				else return NULL;
			}
			buf_len=strlen(input_data[i].val);
			if(buf_len&&!bfound) 
			{
				bfound=1;
				p=(char*)malloc(sizeof(char)*buf_len);
				if(!p) {printf("Out of memory!"); exit(0);}
				strcpy(p,input_data[i].val);
			}
			else if(buf_len&&bfound)
			{
				buf_len=buf_len+strlen(input_data[i].val)+1;
				p=(char*)realloc(p,sizeof(char)*buf_len);
				strcat(p,"\t");
				strcat(p,input_data[i].val);
			}
			else break;
		}
	}
	return p;
}

void AddToSpace(char *str)
{
	register int i;	
	int len=0;
	if(str) len=strlen(str);
	for(i = 0 ; i<len; i++)
		if(str[i] == '+') str[i] = ' ';
}

void CGI::Convert(char *data)
{
	register int i,j;
	for(i = 0,j = 0; data[j]; ++i,++j)
	{
		if((data[i] = data[j]) == '%')
		{
			data[i] = ConvertToHex(&data[j+1]);
			j += 2;
		}
	}
	data[i] = '\0';
}

char* CGI::ReadStdin(FILE *f, char stop)
{
	int wsize,x; char *word;
	wsize = 256; x = 0;
	if((word = (char*)malloc(wsize+1)) == NULL)
		printf("Malloc memory error!");
	while(1)
	{ 
		word[x] = (char)fgetc(f);
		if(x == wsize)
		{
			word[x+1] = '\0';
			wsize += 256;
			word = (char *)realloc(word,sizeof(char)*(wsize+1));
		}
		if((word[x] == stop) || (feof(f)) )
		{
			word[x] ='\0';
			return word;
		}
		++x; 
	}
}


char* CGI::ReadData(char *line,char stop)
{
	int i = 0,j;
	char *word;
	i=strlen(line)+1;
	if((word = new char[i]) == NULL)
	{
		printf("Malloc memory error!"); 
		exit(0);
	}
	for(i = 0; ((line[i])&&(line[i] != stop)); i++) 
		word[i] = line[i];
	word[i] = '\0';
	if(line[i]) ++i;
	j = 0;
	while((line[j++] = line[i++]));
	return word;
}

char CGI::ConvertToHex(char *change)
{
	if((!change[0])||(!change[1])) 
		return 0;

	register char hexdigit;

	hexdigit = (change[0]>='A' ? ((change[0] & 0xdf)-'A') + 10 : (change[0]-'0'));
	hexdigit *= 16;
	hexdigit += (change[1]>='A' ? ((change[1] & 0xdf)-'A') + 10 : (change[1]-'0'));
	return(hexdigit);
}

void    CGI::print_java_err(char *s)
{
	printf("alert('%s');\n",s);
	printf("</script>\n</body>\n</html>");
	exit(1);
}

