#include "zmd5.h"

static	md5_UINT4 _md5_GET_u32(const unsigned char *addr)
{
	return (((((((md5_UINT4)addr[3]<<8)|addr[2])<<8)|addr[1])<<8)|addr[0]);
}

static	void _md5_PUT_u32(md5_UINT4 data,unsigned char *addr)
{
	addr[0]=(unsigned char)data;
	addr[1]=(unsigned char)(data>>8);
	addr[2]=(unsigned char)(data>>16);
	addr[3]=(unsigned char)(data>>24);
}

void	MD5_init(MD5_context *_md5_context)
{
	_md5_context->state[0]=0x67452301;
	_md5_context->state[1]=0xefcdab89;
	_md5_context->state[2]=0x98badcfe;
	_md5_context->state[3]=0x10325476;

	_md5_context->bitcount[0]=0;
	_md5_context->bitcount[1]=0;

	memset(_md5_context->buffer,0,sizeof(_md5_context->buffer));
}

#define F1(x, y, z)	(z ^ (x & (y ^ z)))
#define F2(x, y, z)	F1(z, x, y)
#define F3(x, y, z)	(x ^ y ^ z)
#define F4(x, y, z)	(y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s)					\
	(w += f(x, y, z) + data, w &= 0xffffffff, w = w<<s | w>>(32-s), w += x )

static void MD5_transform(md5_UINT4 _buf[4],const unsigned char _inraw[64])
{
	register md5_UINT4	a=0,b=0,c=0,d=0;
	md5_UINT4		in[16];
	int			i=0;

	for(i=0;i<16;++i)
		in[i]=_md5_GET_u32(_inraw+4*i);

	a=_buf[0];
	b=_buf[1];
	c=_buf[2];
	d=_buf[3];

	MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
	MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
	MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
	MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
	MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
	MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
	MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
	MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
	MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
	MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
	MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
	MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
	MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
	MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
	MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
	MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
	MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
	MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
	MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
	MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
	MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

	_buf[0]+=a;
	_buf[1]+=b;
	_buf[2]+=c;
	_buf[3]+=d;
}

void	MD5_update(MD5_context *_md5_context,unsigned char *_buffer,int _length)
{
	md5_UINT4	_t;

	_t=_md5_context->bitcount[0];
	_md5_context->bitcount[0]=(_t+((md5_UINT4)_length<<3))&0xffffffff;
	if(_md5_context->bitcount[0]<_t)
		_md5_context->bitcount[1]++;
	_md5_context->bitcount[1]+=_length>>29;

	_t=(_t>>3)&0x3F;
	if(_t)
	{
		unsigned char *p=_md5_context->buffer+_t;

		_t=64-_t;
		if(_length<_t)
		{
			memcpy(p,_buffer,_length);
			return;
		}

		memcpy(p,_buffer,_t);
		MD5_transform(_md5_context->state,_md5_context->buffer);
		_buffer+=_t;
		_length-=_t;
	}

	while(_length>=64)
	{
		memcpy(_md5_context->buffer,_buffer,64);
		MD5_transform(_md5_context->state,_md5_context->buffer);
		_buffer+=64;
		_length-=64;
	}

	memcpy(_md5_context->buffer,_buffer,_length);
}

void	MD5_final(MD5_context *_md5_context,unsigned char *_digest)
{
	unsigned	count=0;
	unsigned char	*p;

	/* Compute number of bytes mod 64 */
	count=(_md5_context->bitcount[0]>>3)&0x3F;

	p=_md5_context->buffer+count;
	*p++=0x80;
	count=64-1-count;

	if(count<8)
	{
		memset(p,0,count);
		MD5_transform(_md5_context->state,_md5_context->buffer);
		memset(_md5_context->buffer,0,56);
	}
	else
		memset(p,0,count-8);

	_md5_PUT_u32(_md5_context->bitcount[0],_md5_context->buffer+56);
	_md5_PUT_u32(_md5_context->bitcount[1],_md5_context->buffer+60);

	MD5_transform(_md5_context->state,_md5_context->buffer);
	_md5_PUT_u32(_md5_context->state[0],_digest);
	_md5_PUT_u32(_md5_context->state[1],_digest+4);
	_md5_PUT_u32(_md5_context->state[2],_digest+8);
	_md5_PUT_u32(_md5_context->state[3],_digest+12);
}

int	MD5_hmac(char *_input,int _length,char *_out_md5_hmac)
{
	int		_n=0;
	MD5_context	md5_ctx;
	char		_check_sum[32];
	char		_tmp_STRING[32];

	memset(&md5_ctx,0,sizeof(MD5_context));
	if(_length<1)
		return -1;

	memset(_tmp_STRING,0,sizeof(_tmp_STRING));
	memset(_check_sum,0,sizeof(_check_sum));

	MD5_init(&md5_ctx);
	MD5_update(&md5_ctx,(unsigned char *)_input,_length);
	MD5_final(&md5_ctx,(unsigned char *)_check_sum);

	for(_n=0;_n<16;_n++)
	{
		sprintf(_tmp_STRING,"%02X",(unsigned char)_check_sum[_n]);
		strcat((char *)_out_md5_hmac,_tmp_STRING);
	}
	return 0;
}

/******************************************************************************/

char *_md5_public_KEY_string_(void)
{
    static	char	_public_key_str_[64];
	int	n=0;
#if 0
     025-58464383-GIS.akong-2001@163.com
#endif

	_public_key_str_[n++]='0';
	_public_key_str_[n++]='2';
	_public_key_str_[n++]='5';
	_public_key_str_[n++]='-';
	_public_key_str_[n++]='5';
	_public_key_str_[n++]='8';
	_public_key_str_[n++]='4';
	_public_key_str_[n++]='6';
	_public_key_str_[n++]='4';
	_public_key_str_[n++]='3';
	_public_key_str_[n++]='8';
	_public_key_str_[n++]='3';
	_public_key_str_[n++]='.';
	_public_key_str_[n++]='G';
	_public_key_str_[n++]='I';
	_public_key_str_[n++]='S';

	_public_key_str_[n++]='.';
	_public_key_str_[n++]='a';
	_public_key_str_[n++]='k';
	_public_key_str_[n++]='o';
	_public_key_str_[n++]='n';
	_public_key_str_[n++]='g';
	_public_key_str_[n++]='-';
	_public_key_str_[n++]='2';
	_public_key_str_[n++]='0';
	_public_key_str_[n++]='0';
	_public_key_str_[n++]='1';
	_public_key_str_[n++]='@';
	_public_key_str_[n++]='1';
	_public_key_str_[n++]='6';
	_public_key_str_[n++]='3';
	_public_key_str_[n++]='.';
	_public_key_str_[n++]='c';
	_public_key_str_[n++]='o';
	_public_key_str_[n++]='m';
	_public_key_str_[n++]=0;

	return (char *)_public_key_str_;
}

