
#ifdef CODE32

#include "def.h"
#include "libx2.h"
#include "test.h"
#include "List.h"
#include "PMLoader.h"       //使用常数
#include "Memory.h"

Test::Test():
dbg(Printer(5,20,15,30))
{
    
    
}
Test::~Test()
{
    
    
}
void Test::dumpMemoryData(const MemoryDescriptor & data)
{
    static int count=0;
    char buf[10];
    Util::digitToStr(buf,10,count++);
    Util::printStr(buf);
    Util::printStr("-->");
    Util::digitToStr(buf,10,data.getStart());
    Util::printStr(buf);
    Util::printStr(":");
    Util::digitToStr(buf,10,data.getLimit());
    Util::printStr(buf);
    Util::printStr(":");
    Util::digitToStr(buf,10,data.isAllocable());
    Util::printStr(buf);
    Util::printStr("\n");
}
void Test::dumpMemoryData(const int & data)
{

}
void Test::dumpSMM( SimpleMemoryManager<TreeNode<MemoryDescriptor> > *smm)
{
    char buf[10];
    Util::printStr("DUMP SMM: ");
    Util::digitToStr(buf,10,smm->getLen());
    Util::printStr(buf);Util::printStr(":");
    Util::digitToStr(buf,10,smm->getCurSize());
    Util::printStr(buf);Util::printStr(":");
    Util::digitToStr(buf,10,smm->getLimit());
    Util::printStr(buf);Util::printStr("\n");
}
void Test::dumpMM( MemoryManager &mm)
{
    char buf[10];
    Util::printStr("Head: ");
    Test::dumpMemoryData(mm.getHead()->getData());
    int n=0;
    TreeNode<MemoryDescriptor> *p=mm.getHead()->getSon();
    while(p)
    {
        Util::digitToStr(buf,10,n++);
        Util::printStr("NO_");
        Util::printStr(buf);
        Util::printStr(":");
        Test::dumpMemoryData(p->getData());
        p=p->getNext();
    }

}
DEVEL_AUTHOR(Doug)
DEVEL_AUTHOR(TESTED)
void Test::testMemory()
{
    dbg.putsz("Testing: Memory. \n");

    SimpleMemoryManager<TreeNode<MemoryDescriptor> > smm(PMLoader::FREE_HEAP_START,sizeof(TreeNode<MemoryDescriptor>) * 40 ); //has bug,but to let it smaller makes things works
    //***********test smm
    // Test::dumpSMM(&smm);
    // TreeNode<MemoryDescriptor> * p = smm.getNew();
    // Test::dumpSMM(&smm);
    // TreeNode<MemoryDescriptor> * x = smm.getNew();
    // Test::dumpSMM(&smm);
    // smm.withdraw(p);

    //==============

    MemoryManager mm(&smm,0,0xfffff,0);//0~1MB,用于自身分配,模拟空间分配完毕的情况
    // Test::dumpSMM(&smm);


//    dbg.putsz("done init.\n");
  // dumpMemoryData(mm.getHead()->getData());//OK

    mm.allocFreeStart(0,PMLoader::CODE_START); //this is for preserved area
        dbg.putsz("done init 1.\n");
    mm.allocFreeStart(PMLoader::CODE_START,PMLoader::CODE_LIMIT/100);//for code
    //now it's free to use all the left area
    dbg.putsz("done allocFreeStart.\n");
    
    int *a=mm.mnew(sizeof(int)); 
     *a = 9;
    Util::digitToStr(buf,sizeof(buf),a);//addr
    dbg.putsz(buf);dbg.putsz(":");

    Util::digitToStr(buf,sizeof(buf),*a);
    dbg.putsz(buf);dbg.putsz("\n");

    int *b=mm.mnew(sizeof(int));
     *b=20;
    Util::digitToStr(buf,sizeof(buf),b);//addr
    dbg.putsz(buf);dbg.putsz(":");
    
    Util::digitToStr(buf,sizeof(buf),*b);
    dbg.putsz(buf);
    
    Test::dumpMM(mm);

    dbg.putsz("\n");

    dbg.putsz("End .\n");
    
}
void Test::testList()
{
    char save[10];
    dbg.putsz("Testing: List. \n");
    SimpleMemoryManager<ListNode<int> > smm(PMLoader::FREE_HEAP_START,sizeof(ListNode<int>)*100);
    
    ListNode<int>* node;
    

    LinkedList<int> list(&smm);
    node=list.append(5);
     Util::digitToStr(save,sizeof(save),node->getData());
     dbg.putsz(save);
     dbg.putsz("\n");
     
    
    list.append(7);
    list.append(6);
    
    dbg.putsz("size is:");
    
    Util::digitToStr(save,sizeof(save),list.getSize());
    dbg.putsz(save);
    dbg.putsz("\n");
    
    
    node=list.remove();
    Util::digitToStr(save,sizeof(save),node->getData());
    smm.withdraw(node);
    dbg.putsz(save);
    
    node=list.remove();
    Util::digitToStr(save,sizeof(save),node->getData());
    smm.withdraw(node);
    dbg.putsz(save);
    
    node=list.remove();
    Util::digitToStr(save,sizeof(save),node->getData());
    smm.withdraw(node);
    dbg.putsz(save);
    
    
    list.free();
    Util::digitToStr(save,sizeof(save),smm.getCurSize());
    dbg.putsz("\nFree:");
    dbg.putsz(save);
    
    dbg.putsz("\nEnd.\n");
    
    
    
}
void Test::testQueue()
{
     dbg.putsz("Testing: Queue. \n");
    
    char save[10];
    unsigned char buf[100];
    Queue<unsigned char> q(buf,sizeof(buf));
    unsigned char p='a';
    q.add(p);
    q.add('b');
    q.add('v');
    Util::digitToStr(save,sizeof(save),q.size());
    dbg.putsz("Current size:");dbg.putsz(save);dbg.putc('\n');
    p=q.remove();
    dbg.putsz("Get : ");
    dbg.putc(p);
    p=q.remove();
    dbg.putc(p);
    p=q.remove();
    dbg.putc(p);
}
void Test::run()
{
    //testQueue();
    //testList();
    testMemory();
}

#endif