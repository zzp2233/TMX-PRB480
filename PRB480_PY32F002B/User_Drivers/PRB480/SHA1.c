#include <stdio.h>
#include <string.h>
#include "SHA1.h"

unsigned char sha_result[200];
unsigned char count_time = 0;

typedef union {
    unsigned char c[64];
    unsigned long l[16];
} CHAR64LONG16;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/*   blk0()   and   blk()   perform   the   initial   expand.   */
/*   I   got   the   idea   of   expanding   during   the   round   function   from   SSLeay   */
#ifdef LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | (rol(block->l[i], 8) & 0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/*   (R0+R1),   R2,   R3,   R4   are   the   different   operations   used   in   SHA1   */
#define R0(v, w, x, y, z, i)                                     \
	z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);

#define R1(v, w, x, y, z, i)                                    \
	z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);

#define R2(v, w, x, y, z, i)                            \
	z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
	w = rol(w, 30);

#define R3(v, w, x, y, z, i)                                          \
	z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
	w = rol(w, 30);

#define R4(v, w, x, y, z, i)                            \
	z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
	w = rol(w, 30);


static void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);

//����Ե���512�����ݿ����80�ֵ�ѭ�����㡣���Ĳ���
/*
   ��Ϣ��չ�����ȣ���512λ������飨16��32λ���֣���չ����80��32λ���֣��Թ�80������ʹ��
	 
	 80��ѭ�����㣺ÿ�����㶼���5���Ĵ�����A,B,C,D,E����ֵ���и��¡�ÿ20��Ϊһ�飬ÿ��
	 ʹ�ò�ͬ�ķ������߼���ft���ͳ�����Kt����
	 
	 �м����ϲ���80��������ɺ󣬽����ּ���õ���A,B,C,D,E��ֵ����������ĳ�ʼֵ����
	 ��һ�ֽ���ʱ��״̬����ӣ��õ��µĹ�ϣ״̬

*/

/*   Hash   a   single   512-bit   block.   This   is   the   core   of   the   algorithm.   */
static void SHA1Transform(unsigned long state[5], unsigned char buffer[64])
{

    unsigned long a, b, c, d, e, i;
    unsigned char sha_data[20];
    CHAR64LONG16* block;
    count_time++;
#ifdef SHA1HANDSOFF
    static unsigned char workspace[64];
    block = (CHAR64LONG16*)workspace;
    memcpy(block, buffer, 64);
#else
    block = (CHAR64LONG16*)buffer;
#endif

    /*   Copy   context-> state[]   to   working   vars   */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

   /* ��ɵľ���RFC�ĵ��е�H0��H4��ֵ��ABCDE�Ĳ���������������80������Ĵ��롣ÿ20��Ϊһ�飬�������� */
	 /* ��һ��Ƚ����⣬ʹ����R0��R1�����꺯������ԭ��ǰ���Ѿ������ˡ���Ϊ��0~15�������16~79�������ʱ����Ϣ��M(i)���ֿ�W(i)��ת���ǲ�һ���ġ�
	   �����20~39�֣�40~59�֣�60~79�־�������ʹ�õ�R2��R3��R4�������� */
	 /*   4   rounds   of   20   operations   each.   Loop   unrolled.   */
    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);

    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);

    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);

    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);

    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);

    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);

    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);

    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);

    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);

    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);

    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);

    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);

    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);

    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);

    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);

    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);

    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);

    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);

    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);

    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);

    /* ��ɵľ��Ǹ��»�����H0��H4�����ݡ�Ȼ���a��e���Ϊ0 */
	/*   Add   the   working   vars   back   into   context.state[]   */
//    state[0] += a;
//    state[1] += b;
//    state[2] += c;
//    state[3] += d;
//    state[4] += e;
		
		state[0] = a;
    state[1] = b;
    state[2] = c;
    state[3] = d;
    state[4] = e;
		
    for (i = 0; i < 4; ++i)
    {
        sha_data[i] = (state[4] >> (i * 8)) & 0x000000ff;
        sha_data[i + 4] = (state[3] >> (i * 8)) & 0x000000ff;
        sha_data[i + 8] = (state[2] >> (i * 8)) & 0x000000ff;
        sha_data[i + 12] = (state[1] >> (i * 8)) & 0x000000ff;
        sha_data[i + 16] = (state[0] >> (i * 8)) & 0x000000ff;
    }

    if (count_time == 1)memcpy(sha_result, sha_data, 20);//��ż�����̽��
    else if (count_time == 2)memcpy(&sha_result[20], sha_data, 20);
    else if (count_time == 3)memcpy(&sha_result[40], sha_data, 20);
    else if (count_time == 4)memcpy(&sha_result[60], sha_data, 20);
    else if (count_time == 5)memcpy(&sha_result[80], sha_data, 20);
    else if (count_time == 6)memcpy(&sha_result[100], sha_data, 20);
    else if (count_time == 7)memcpy(&sha_result[120], sha_data, 20);
    else if (count_time == 8)memcpy(&sha_result[140], sha_data, 20);

    /*   Wipe   variables   */
    a = b = c = d = e = 0;

}

/*   SHA1Init   -   Initialize   new   context   */

/*
ʹ��5��32λ�ļĴ������洢�͸��¹�ϣ������м�״̬���㷨ʹ��5��32λ�ļĴ������洢��
���¹�ϣ������м�״̬

*/
void SHA1Init(SHA1_CTX* context)
{
    /*   SHA1   initialization   constants   */
    context->state[0] = 0x67452301;

    context->state[1] = 0xEFCDAB89;

    context->state[2] = 0x98BADCFE;

    context->state[3] = 0x10325476;

    context->state[4] = 0xC3D2E1F0;

    context->count[0] = context->count[1] = 0;
}

void SHA1init(SHA1_CTX* context, unsigned int* value)
{
    /*   SHA1   initialization   constants   */
    context->state[0] = value[0];

    context->state[1] = value[1];

    context->state[2] = value[2];

    context->state[3] = value[3];

    context->state[4] = value[4];

    context->count[0] = context->count[1] = 0;
}

/*   Run   your   data   through   this.   */

/*
1.ά��λ��������context->count�������ھ�ȷ��¼��ĿǰΪֹ�Ѵ������Ϣ����λ��
2.��仺���������ݱ��ֿ鴦���ÿ��512��64�ֽڣ��������᳢������һ��64�ֽڵ��ڲ�������context->buffer
3.���ñ任������ÿ�����������������ﵽ64�ֽڣ����ͻ����SHA1Transform���������512λ�����ݿ���к��ĵĹ�ϣ����
*/
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len)
{
    unsigned int i, j;

    /*   j>>3��õľ����ֽ�����j = (j >> 3) & 63�õ��ľ��ǵ�6λ��ֵ��Ҳ���Ǵ���64���ֽڣ�512λ�����ȵ���Ϣ������Ϊ����ÿ�ν��м��㶼�Ǵ���512λ����Ϣ���ݡ� */
		j = (context->count[0] >> 3) & 63;

    /* context->count[ ]�洢������Ϣ�ĳ��ȣ�����context->count[0]�Ĵ洢��Χ�Ĳ��ִ洢��context->count[1]�С�len<<3����len*8����˼����Ϊlen�ĵ�λ���ֽڣ���context->count[ ]�洢�ĳ��ȵĵ�λ��λ������Ҫ����8�� if ((context->count[0] += len << 3) < j) ����˼����˵�������len*8��λ��context->count[0]����ˣ���ô��Ҫ��context->count[1]++����λ��
	len<<3�ĵ�λ��λ��len>>29��len<<3 >>32����ʾ�ľ���len��Ҫ�洢��context->count[1]�еĲ��֡� */
    if ((context->count[0] += len << 3) < (len << 3))
        context->count[1]++;

    context->count[1] += (len >> 29);

   /* ���j+len�ĳ��ȴ���63���ֽڣ��ͷֿ������ÿ64���ֽڴ���һ�Σ�Ȼ���ٴ�������64���ֽڣ��ظ�������̣������ֱ�ӽ����ݸ��ӵ�bufferĩβ */
		if ((j + len) > 63)
    {
        memcpy(&context->buffer[j], data, (i = 64 - j));		/* i=64-j��Ȼ���data�и���i���ֽڵ����ݸ��ӵ�context->buffer[j]ĩβ��Ҳ����˵��buffer�ճ���64���ֽ� */
        SHA1Transform(context->state, context->buffer);		/* ִ��SHA1Transform()����ʼһ����ϢժҪ�ļ��� */
       /* ÿ64���ֽڴ���һ�� */
        for (; i + 63 < len; i += 64)
        {
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else
    {
        i = 0;
    }

    /* ���ǰ���if����������ôҲ����˵ԭʼ����context->buffer�����µ�����data�ĳ��Ȼ������Դճ�64���ֽڣ�����ֱ�Ӹ�����data�����ˡ��൱�ڣ�memcpy(&context->buffer[j], &data[i], 0);
	���ǰ���if��������ôj�ǵ���0�ģ��� i ��ָ���ƫ��λ���� (�� len/64����64,len)֮�䡣 ��   ����ʾ����ȡ����*/

    memcpy(&context->buffer[j], &data[i], len - i);
}


/*
1.������λ����������Ϣĩβ���һ�����ص�1��Ȼ�������ɸ����ص�0��ֱ����Ϣ�ĳ��������
512ȡģ����������448λ
2.��ӳ�����Ϣ�������֮�󣬸���һ��64λ���ֶΣ����ԭʼ������Ϣ��λ���ȣ���
3.�������տ飺������������ͨ���ᴥ�������һ�����ݿ飨���ܰ������λ�ͳ�����Ϣ����
SHA1Transform����
4.���ժҪ����󣬽������ĵ�5��32λ��״̬�Ĵ������������ת��Ϊ20���ֽڵ����У�
��Ϊ���յ�160λSHA-1��ϣֵ���洢��digest�����С�
5.
*/


/*   Add   padding   and   return   the   message   digest.   */
void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
    unsigned long i;
//		unsigned long j;
    unsigned char finalcount[8];
    for (i = 0; i < 8; i++)
    {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255); /*   Endian   independent   */
    }
   /* ����ʱ�������ֽ�Ϊ��λ�ģ�����1���ֽڣ����64���ֽڡ����ҵ�һλҪ���1�����涼���0�������õ�һ����Ϣ��������Ҫ�������һ���ֽڵ�10 000 000.SHA1Update() ����������ɵ�������䣨���ӣ����� */
		SHA1Update(context, (unsigned char*)"\200 ", 1);

   /* ѭ����������ģ512�Ƿ���448ͬ�ࡣ���������������ȫһ���ֽ�0�� */
	/* ʹ�� while ((context->count[0] & 511) != 448) ò�Ƹ����ʡ����ǣ�504����λȫ0,511����λȫ1��context->count�д洢������Ϣ�ĳ��ȣ����ĵ�λ�ǣ�λ��ǰ�������ᵽ�����ǵ����������ֽ����洢�ģ�����context->count[ ]�е����ݿ϶���8�����������Ժ���λ�϶���000�����Բ�����000&000������000&111��������0�� */
		while ((context->count[0] & 504) != 448)
    {
        SHA1Update(context, (unsigned char*)"\0 ", 1);
    }

   /* �⽫����SHA1Transform()�����ĵ��ã��ú����Ĺ��ܾ��ǽ������㣬�ó�160λ����ϢժҪ��message digest����������context-state[ ]�У���������SHA-1�㷨�ĺ��� */
		SHA1Update(context, finalcount, 8); /*   Should   cause   a   SHA1Transform()   */

   /* �����ⲽת������ϢժҪת���ɵ��ֽ����С��ô��������;��ǣ���context-state[5]�д����20���ֽڣ�5��4�ֽڣ�����ϢժҪȡ��������洢��20�����ֽڵ�����digest�С����Ұ������洢����֮ǰ����context->count[ ]��finalcount[ ]ת����˼·��ͬ�� */
		for (i = 0; i < 20; i++)
    {
        digest[i] = (unsigned char)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
    /*   Wipe   variables   */
    i = 0;//j = 0;
    memset(context->buffer, 0, 64);
    memset(context->state,  0, 20);
    memset(context->count,  0, 8);
    memset(&finalcount, 0, 8);

#ifdef SHA1HANDSOFF /*   make   SHA1Transform   overwrite   it 's   own   static   vars   */
    SHA1Transform(context->state, context->buffer);
#endif
}

unsigned char hash[20];
void SHA1(unsigned char* data, unsigned int len)//400ns���Ҽ���һ������
{
    //unsigned char i;
    SHA1_CTX ctx;
    count_time = 0;
   
    SHA1Init(&ctx);
    SHA1Update(&ctx, data, len); 
    SHA1Final(hash, &ctx);
}

void SHA1_change(unsigned char* chushizhi, unsigned char* data, unsigned int len)
{
    unsigned char i;
    SHA1_CTX ctx;
    unsigned int value_init[5] = { 0 };
    count_time = 0;
    for (i = 0;i < 5;i++)value_init[i] = chushizhi[i * 4 + 3] << 24 | chushizhi[i * 4 + 2] << 16 | chushizhi[i * 4 + 1] << 8 | chushizhi[i * 4];
   
    SHA1init(&ctx, value_init);
    SHA1Update(&ctx, data, len);
    SHA1Final(hash, &ctx);
}

