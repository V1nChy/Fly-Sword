#define ClassName L"这个是类名"
#define WindowTitle L"这个是标题"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define BULLET_NUM 30
#define SMALLDRAGON_NUM 7
#define STORM_NUM 5
#define ROLE 1
#define SMALLDRAGON 2
#define SWORDBLADE 3
#define EXPLODE1 11
#define EXPLODE2 12
#define STORM 13
#define BOSS 5
#define SCORE_BOSS 3
#define SCORE_FINISH 200
struct SwordBullets
{
	int x, y;
	bool exist;
};
struct SmallDragon
{
	int x;
	int y;
	int state;
	int direct;
	bool exist;
	DWORD tNow;
};
struct Boss
{
	int x, y;
	int state;
	int skill;
	int skill_state;
	int life;
	int direct;
	bool exist;
	DWORD tNow;
	DWORD tKill;
};
struct Role
{
	int x, y;
	int state;
	int score;
	int life;
	bool skill;
	bool exist;
	DWORD tNow;
};
struct Explode
{
	int x, y;
	int direct_x, direct_y;
	bool exist;
	int kind;
	int state;
	DWORD tNow;
};
struct Hole
{
	int x, y;
	bool exist;
};

template <class T>
class Node
{
public:
	T Data;
	Node<T> *next;
	Node<T> *pre;
	Node()
	{
		pre = next = NULL;
	}
	Node(const T& x)
	{
		Data = x;
		pre = next = NULL;
	}
};

template <class T>
class List
{
	Node<T> *head;
	int size;
public:
	List();
	void Insert(const T& x);
	Node<T>* Delete(Node<T>* d);
	void DeleteAll();
	int Return_size();
	Node<T>* Return_head();
};

template<class T>
List<T>::List()
{
	head = new Node<T>();
	head->pre = head->next = head;
	size = 0;
}

template<class T>              //尾插入链表
void List<T>::Insert(const T&x)
{
	Node<T> *p =new Node<T>(x);
	p->next = head;
	p->pre = head->pre;
	head->pre->next = p;
	head->pre = p;
	size++;
}

template<class T>              //删除节点d
Node<T>* List<T>::Delete(Node<T>* d)
{
	Node<T> *p = d->next;
	d->pre->next = d->next;
	d->next->pre = d->pre;
	delete d;
	size--;
	return p;
}


template<class T>
void List<T>::DeleteAll()
{
	size = 0;
	Node<T>* p = head->next;
	Node<T>* old;
	while (p != head)
	{
		old = p;
		p = p->next;
		delete old;
	}
	head->pre = head->next = head;
}
template<class T>              //返回节点数
int List<T>::Return_size()
{
	return size;
}

template<class T>              //返回头指针
Node<T>* List<T>::Return_head()
{
	return head;
}