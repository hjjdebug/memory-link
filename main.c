#include <stdio.h>
/* 简单的内存链表的使用， 可以自由分配，释放，但只分配一次，因为回收并不释放给系统，
 * 而是交给链表，第二次分配时直接从链表中获取。
 */
extern void * my_malloc(int size);
extern void my_free(void *p,int size);
int main()
{
	void *p1=my_malloc(100);
	void *p2=my_malloc(100);
	printf("%-10s: p1 is %p,p2 is %p\n","first",p1,p2);
	my_free(p1,100);
	my_free(p2,100);
	p1=my_malloc(100);
	p2=my_malloc(100);
	printf("%-10s: p1 is %p,p2 is %p\n","second",p1,p2);
	my_free(p1,100);
	my_free(p2,100);
	return 0;
}

