#ifdef CODE16
    __asm__(".code16gcc \n\t");
#elif defined CODE32
    __asm__(".code32 \n\t");
#endif


#include "libx2.h"
#include "def.h"


int Util::x=0;
int Util::y=0;
const int Util::MODE_FL_ON=0x80,
        Util::MODE_FL_OFF=0x7f,
        Util::MODE_BG_RED=0b0100000,
        Util::MODE_BG_GREEN=0b0010000,
        Util::MODE_BG_BLUE=0b0001000,
        Util::MODE_BG_WHITE=0b0111000,
        Util::MODE_BG_RG=0b0110000,
        Util::MODE_BG_RB=0b0101000,
        Util::MODE_BG_BG=0b0011000,
        Util::MODE_BG_BLACK=0b0000000,
        Util::MODE_FG_RED=0b0000100,
        Util::MODE_FG_GREEN=0b0000010,
        Util::MODE_FG_BLUE=0b0000001,
        Util::MODE_FG_WHITE=0b0000111,
        Util::MODE_FG_RG=0b0000110,
        Util::MODE_FG_RB=0b0000101,
        Util::MODE_FG_BG=0b0000011,
        Util::MODE_FG_BLACK=0b0000000,
        Util::MODE_COMMON=Util::MODE_FL_OFF & Util::MODE_BG_BLACK | Util::MODE_FG_WHITE;
const int Util::SCREEN_X=25,Util::SCREEN_Y=80;
const int Util::SEG_CURRENT=0x10000;

//const int Util::SEG_CURRENT =    0x10000;//运行期常数，有时我需要一个编译期常数,那就在g++的命令行中定义之-D
int Util::videoSelector=
#ifdef CODE16
    0xb800;
#elif defined CODE32
    0b1000; //指向videoSelector
#endif


Util::Util()
{
    Util::printStr(">>>Init");
    Util::newLine();    
}

Util::~Util()
{
    Util::printStr(">>>Destroy");
    Util::newLine();
}
void Util::test()
{
    Util::printStr(">>>test");
    Util::newLine();
}

void Util::printStr(const char* str_addr,int mode)
{
    while(*str_addr)
    {
        Util::printChar(*str_addr++,mode);
    }
}
void Util::printChar(char ch,int mode)
{
    if(ch == '\n')
    {
        Util::newLine();
        return;
    }else if(ch == '\r')
    {
        Util::y = 0;
        return;
    }else if(ch=='\b')
    {
        if(Util::y>0)
            Util::y--;
        return;
    }
    
    if(Util::y==80) //为了防止某些情况下两次newLine，比如打印完最后一个字符时。所以把这个移动到这里
    {
        Util::newLine();
    }
    int Lpos = Util::x * 80*2 + Util::y*2;//25 * 80
    __asm__ __volatile__(
    "push %%es\n\t"
    "mov %%dx,%%es\n\t" //视频缓冲区
    "movb %%al,%%es:(%%bx) \n\t"
    "inc %%ebx \n\t"
    "movb %%cl,%%es:(%%bx) \n\t"
    "pop %%es \n\t"
    :
    :"a"(ch),"b"(Lpos),"c"(mode),"d"(Util::videoSelector)
    :"memory"
    );

    Util::y ++;
    
}
void Util::newLine()
{
   
    Util::y = 0;
    Util::x = (Util::x+1)%25;
}
void Util::insertMark(int marker)
{
    __asm__(
     "nop \n\t"
    );
}
 int Util::get(int seg,int off)
 {
     ENTER_DS(seg,s);
     int rt;
     __asm__ __volatile__(
        "movl %%ds:(%%ebx),%%eax \n\t"
       :"=a"(rt)
       :"b"(off),"a"(seg)
       :
     );
    LEAVE_DS(seg,s);
     return rt;
 }
void Util::setb(int seg,int off,int byte)
{
    ENTER_DS(seg,s);
    __asm__ __volatile__(
        "movl 4+4*2(%%ebp),%%ebx \n\t"
        "movb 4+4*3(%%ebp),%%al \n\t"
        "movb %%al,%%ds:(%%ebx) \n\t"
        :
        :
        :"ebx","eax","memory"
    );
    LEAVE_DS(seg,s);
}
void Util::setw(int seg,int off,int halfWorld)
{
    ENTER_DS(seg,s);
    __asm__ __volatile__(
        "movl 4+4*2(%%ebp),%%ebx \n\t"
        "movw 4+4*3(%%ebp),%%ax \n\t"
        "movw %%ax,%%ds:(%%ebx) \n\t"
        :
        :
        :"ebx","eax","memory"
    );
    LEAVE_DS(seg,s);
}
void Util::setl(int seg,int off,int word)
{
    ENTER_DS(seg,s);
    __asm__ __volatile__(
        "movl 4+4*2(%%ebp),%%ebx \n\t"
        "mov 4+4*3(%%ebp),%%eax \n\t"
        "mov %%eax,%%ds:(%%ebx) \n\t"
        :
        :
        :"ebx","eax","memory"
    );
    LEAVE_DS(seg,s);
}

void Util::memcopy(int srcSeg,int srcOff,int dstSeg,int dstOff,int len)
{   
    ENTER_ES(dstSeg,s2);
    ENTER_DS(srcSeg,s1);

    __asm__( //ds:si --> es:di
    "push %esi \n\t"
    "push %edi \n\t"
    "mov 4+4*2(%ebp),%esi \n\t"
    "mov 4+4*4(%ebp),%edi \n\t"
    "mov 4+4*5(%ebp),%ecx \n\t"
    "cld \n\t"
    "rep movsb \n\t"
    "pop %edi \n\t"
    "pop %esi \n\t"
    );
    
    LEAVE_DS(srcSeg,s1);
    LEAVE_ES(dstSeg,s2);
}
void Util::clr()
{
    Util::setCursor(0,0);
    for(int i=0;i!=Util::SCREEN_X;i++)
        for(int j=0;j!=Util::SCREEN_Y;j++)
            Util::printChar(' ');
    Util::setCursor(0,0);
}
void Util::setCursor(int x,int y)
{
    Util::x =x % Util::SCREEN_X;
    Util::y =y % Util::SCREEN_Y;
}
//======================仅32位=============
#ifdef CODE32
short Util::makeSel(int index,int dpl=0b00,int from=0)
{
    return (short)(
        ((index & 0x1fff) << 3) | 
        ((from & 0x1) << 2) | 
        (dpl & 0b11)
        );
}
void Util::lidt(short len,int address)
{
        __asm__(
        "movw 4+4*1(%ebp),%ax \n\t"
        "movw %ax,4+4*1+2(%ebp) \n\t"
        "lidt 4+4*1+2(%ebp) \n\t"
    );
}
void Util::lgdt(short len,int address)
{
        __asm__(
        "movw 4+4*1(%ebp),%ax \n\t"
        "movw %ax,4+4*1+2(%ebp) \n\t"
        "lgdt 4+4*1+2(%ebp) \n\t"
    );
}
char Util::getCPL()
{
   __asm__(
    "mov %cs,%ax \n\t"
    "and $0b11,%al \n\t"
   );
}
char Util::getDPL(int sel)
{
    __asm__(
    "movw 4+4*1(%ebp),%ax \n\t"
    "and $0b11,%al \n\t"
    );
}
void Util::changeCPL(int eip,int cs,int eflags,int esp,int ss)
{
    __asm__(
      "add $4*2,%esp \n\t"
     "iret \n\t"
    );
}
int Util::getEflags()
{
    __asm__(
     "pushfl \n\t"
     "popl %eax \n\t"
    );
}
int Util::digitToStr(char* save,unsigned int space,int n)
{
    int sign=1;
    unsigned int index=0;
    if(index==space)return 0;
    save[index]=0;//结束字符串
    index++;
    
    if(n<0)
    {
       n= -n;
       sign = -1;
    }
    if(n==0)
    {
        if(index==space)return 0;
        save[index]='0';
        index++;
    }else{
        while(n>0)
        {
            if(index==space)return 0;
            save[index]=n%10+'0';
            n/=10;
            index++;
        }
    }
    if(sign==-1)
    {
        if(index==space)return 0;
        save[index]='-';
        index++;
    }
    char temp;
    unsigned int i=0,j=index-1;
    while(i<j)
    {
        temp=save[i];
        save[i]=save[j];
        save[j]=temp;
        i++;
        j--;
    }
    return index;
}
int Util::digitToHex(char* save,unsigned int space,unsigned int n)
{
    unsigned int index=0;
    if(index==space)return 0;
    save[index]=0;//结束字符串
    index++;
    if(n==0)
    {
        if(index==space)return 0;
        save[index]='0';
        index++;
    }else{
        while(n>0)
        {
            if(index==space)return 0;
            int temp=n%16;
            if(temp<10)
            {
                save[index]=temp+'0';
            }else{
                save[index]=temp-10+'a';
            }
            n/=16;
            index++;
        }
        
    }
    char temp;
    int i=0,j=index-1;
    while(i<j)
    {
        temp=save[i];
        save[i]=save[j];
        save[j]=temp;
        i++;
        j--;
    }
    return index;
    
}
int Util::strcmp(const char* a,const char *b)
{
   int i=0;
   while( *(a+i) && *(b+i) && *(a+i)==*(b+i))i++;
   return *(a+i) - *(b+i);
}
int Util::strlen(const char *a)
{
    int i=0;
    while(*(a+i))i++;
    return i;
}
int Util::strcopy(const char *src,char *dst,int len)
{
    int i=0;
    while(i!=len - 1 && *(src+i))
    {
        *(dst+i)=*(src+i);
        i++;
    }
    *(dst+i)=0;
    return i;
}
//=================class : SimpleCharRotator
const char SimpleCharRotator::rotateShapes[12]={'_','\b',0,'\\','\b',0,'|','\b',0,'/','\b',0};
SimpleCharRotator::SimpleCharRotator(int x,int y,int attr,int direction):X(x),Y(y),Attr(attr),Status(0),Direction(direction)
{
    
}
SimpleCharRotator::~SimpleCharRotator()
{
    
}
void SimpleCharRotator::setPosition(int x,int y)
{
    this->X = x;
    this->Y = y;
}
void SimpleCharRotator::setAttr(int mode=Util::MODE_COMMON)
{
    this->Attr=mode;
}
void SimpleCharRotator::run()
{
    while(1)
    {
        Util::setCursor(this->X,this->Y);
        //调用0x24中断打印字符
        Util::insertMark(0x5561);
        CALL_INT_3(0x24,c,Util::SEG_CURRENT,b,&SimpleCharRotator::rotateShapes[this->Status],d,this->Attr);
        this->Status = (this->Status*this->Direction + 3) % (sizeof(SimpleCharRotator::rotateShapes)/sizeof(char));
    }
}

//=================class:Printer
const int Printer::SCREEN_MAX_X=25,
        Printer::SCREEN_MAX_Y=80; //起始const static 也算是编译期常数，
Printer::Printer(unsigned int x0,unsigned int y0,unsigned int rows,unsigned int cols,int mode):
rows(rows>Printer::SCREEN_MAX_X?Printer::SCREEN_MAX_X:rows),
cols(cols>Printer::SCREEN_MAX_Y?Printer::SCREEN_MAX_Y:cols),
x0(x0+rows>=Printer::SCREEN_MAX_X?0:x0),
y0(y0+cols>=Printer::SCREEN_MAX_Y?0:y0),
x(0),y(0),
mode(mode)
{
     
}
Printer::~Printer()
{
    
}

void Printer::move(int n)
{
    this->y+=n;
    while(this->y < 0)
    {
        this->y+=this->cols;
        this->x--;
        if(this->x == -1)
        {
            this->x=this->rows - 1;
        }
    }
    while(this->y >= this->cols)
    {
        this->y -= this->cols;
        this->x++;
        if(this->x==this->rows)
        {
            this->x = 0 ;
        }
    }
    
    
    
}
void Printer::putc(int chr)
{
    int oldes;
    Util::enterEs(Util::videoSelector,oldes);
    switch(this->specailCharProcessor(chr))
    {
        case 0:
            this->__putc(chr);
            this->move(1);
            break;
        case 1:
        case -1:
        default:
            break;
    }
    Util::leaveEs(oldes);
}
void Printer::putsz(char* str)
{
    int oldes;
    char *p=str;
    Util::enterEs(Util::videoSelector,oldes);
    int rt;
    while((rt=this->specailCharProcessor(*p))!=-1)
    {
        if(rt==0)
        {
            this->__putc(*p);
            this->move(1);
        }
        p++;
    }
    Util::leaveEs(oldes);

}
void Printer::putsn(char *str,int n)
{
    int oldes;
    char *p=str;
    Util::enterEs(Util::videoSelector,oldes);
    int rt;
    int i=0;
    while((rt=this->specailCharProcessor(*p))!=-1 && i!=n )
    {
        if(rt==0)
        {
            this->__putc(*p);
            this->move(1);
        }
        p++;
        i++;
    }
    Util::leaveEs(oldes);
}
void Printer::setPos(int x,int y)
{
    this->x=x % this->rows;
    this->y=y % this->cols;
}
void Printer::setMode(int mode)
{
    this->mode=mode;
}
int Printer::specailCharProcessor(int chr)
{
    int rt=1;
    switch(chr)
    {
        case '\n':
            this->x = (this->x + 1) % this->rows;
            this->y = 0;
            break;
        case '\r':
            this->y = 0;
            break;
        case '\b':
            this->move(-1);
            break;
        case '\t':
            this->move(4);
            break;
        case '\0':
            rt=-1;
            break;
        default://
            rt=0;
            break;
    }
    return rt;
}

int Printer::getPos()
{
    return (this->x0+this->x)*Printer::SCREEN_MAX_Y*2+(this->y0+this->y)*2;
}
void Printer::__putc(int chr)
{
    __asm__ __volatile__(
    "mov %%al,%%es:(%%edi) \n\t"
    "mov %%dl,%%es:1(%%edi) \n\t"
    :
    :"a"(chr),"D"(this->getPos()),"d"(this->mode)
    :
    );
}
void Printer::clr()
{
    this->setPos(0,0);
    for(int i=0;i!=this->rows;i++)
    {
        for(int j=0;j!=this->cols;j++)
        {
            this->putc(' ');
        }
    }
    this->setPos(0,0);
}
//==========class : String
String::String(const char* str)
{
    
}
String::~String()
{
    
}

int String::size()
{
    
}
char String::get(int index)
{
    
}
void String::set(int index,int ch)
{
    
}

String String::valueOf(int n)
{
    
}


#endif  //CODE32
#ifdef CODE16
//======================仅16位===========
//ebp + 4 = 返回地址
//ebp + 4*n : n>0,第n个参数
int Util::readSectorsCHS(int dstSeg,int dstOff,int driver,int cylinder,int head,int startSec,int numSecs)
{
    int isCarried;
    __asm__ __volatile__(
    "push %%es\n\t"
    "mov 4+4*1(%%ebp),%%eax \n\t"
    "movw %%ax,%%es\n\t"
    "movw 4+4*2(%%ebp),%%bx\n\t"
    "movb 4+4*3(%%ebp),%%dl \n\t"
    "movb 4+4*5(%%ebp),%%dh \n\t"
    "movw 4+4*4(%%ebp),%%cx \n\t"
    "shl   $6,%%cx \n\t"
    "addb 4+4*6(%%ebp),%%cl \n\t"
    "movb $0x02,%%ah \n\t"
    "movb 4+4*7(%%ebp),%%al \n\t"
    "int $0x13 \n\t"
    "pop %%es \n\t"
    "xor %%eax,%%eax \n\t"
    "jc error \n\t"
    "mov $1,%%eax \n\t"
    "error:\n\t"
    :"=a"(isCarried)
    :
    :"memory","cc"
    );
    return isCarried;
}
int Util::readSectors(int dstSeg,int dstOff,int driver,int LBAStart,int numSecs)
{
    return Util::readSectorsCHS(dstSeg,dstOff,driver,LBAStart/36,(LBAStart - LBAStart/36*36)/18,(LBAStart%18) + 1,numSecs);
}
#endif