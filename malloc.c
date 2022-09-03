#include <stdio.h>
#include <stdlib.h>

//小内存分配管理程序, 参考了linux0.11代码
struct mem_desc {	
	void				*addr;	// 内存地址
	struct mem_desc 	*next;		// 下一个描述符
};

struct _bucket_dir {	
	int			size;
	struct mem_desc		*freeptr;	//contains a linked list of free mem descriptor blocks
	struct mem_desc		*usedptr;		//contains a linked list of used mem descriptor blocks
};

/*
 * Note that this list *must* be kept in order.
 */
struct _bucket_dir bucket_dir[] = {
	{ 16,		0,		0},
	{ 32,		0,		0},
	{ 64,		0,		0},
	{ 128,		0,		0},
	{ 256,		0,		0},
	{ 512,		0,		0},
	{ 1024,		0,		0},
	{ 2048,		0,		0},
	{ 4096,		0,		0},
	{ 0,		0,		0}
};   /* End of list marker */


void panic(char *msg)
{
	printf("%s\n",msg);
	exit(1);
}

void *my_malloc(int len)
{
	/*
	 * First we search the bucket_dir to find the right bucket
	 * for this request.
	 */
	struct _bucket_dir	*bdir;
	for (bdir = bucket_dir; bdir->size; bdir++)
		if (bdir->size >= len)
			break;
	if (!bdir->size) {
		printf("malloc called with impossibly large argument (%d)\n", len);
		panic("malloc: bad arg");
	}
	/*
	 * then we find a mem block,
	 * If we didn't find a mem block with free space, 
	 * then we'll allocate a new one.
	 */
	if(!bdir->freeptr)
	{
		struct mem_desc *p=malloc(sizeof(struct mem_desc)); //分配描述符
		if (!p)
			panic("Out of memory in malloc()");
		p->addr = malloc(bdir->size);		//分配内存
		if (!p->addr)
			panic("Out of memory in malloc()");
		p->next = 0;
		bdir->freeptr=p;
	}
	struct mem_desc *tmp_ptr = bdir->freeptr->next; //保留空闲指针的下一个指针，此为新的空闲指针
	//调整该桶的使用指针
	bdir->freeptr->next = bdir->usedptr; /* OK, link it in! */
	bdir->usedptr = bdir->freeptr;			
	//调整该桶空闲指针
	bdir->freeptr = tmp_ptr;
	void *retval = (void *) bdir->usedptr->addr; 
	return(retval);
}

/*
 * Here is the free routine.  If you know the size of the object that you
 * are freeing, then my_free() will use that information to speed up the
 * search for the bucket descriptor.
 * 
 * We will #define a macro so that "free(x)" is becomes "my_free(x, 0)"
 */
void my_free(void *obj, int size)
{
//	void		*page;
	struct _bucket_dir	*bdir;
	struct mem_desc	*mdesc, *prev;
	mdesc = prev = 0;
	/* Now search the buckets looking for that size */
	for (bdir = bucket_dir; bdir->size; bdir++) {
		prev = 0;
		/* If size is zero then this conditional is always false */
		if (bdir->size < size)
			continue;
		for (mdesc = bdir->usedptr; mdesc; mdesc = mdesc->next) {
			if (mdesc->addr == obj) 
				goto found;
			prev = mdesc;
		}
	}
	panic("Bad address passed to my_free()");
found:
	//调整 使用链, link it out the chain
	if (prev)				
		prev->next = mdesc->next;
	else {
		if (bdir->usedptr != mdesc)
			panic("malloc bucket chains corrupted");
		bdir->usedptr = mdesc->next;
	}
	//调整空闲链, link it in freeptr chain
	mdesc->next = bdir->freeptr;
	bdir->freeptr=mdesc;
	return;
}

