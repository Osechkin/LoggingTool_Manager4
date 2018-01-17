#ifndef IO_CONTAINERS_H
#define IO_CONTAINERS_H

#include <stdlib.h>


template <class Item>
class STACK
{
public:
    STACK()
    {
        head = 0;
        cnt = 0;
    }

    ~STACK()
    {
        while (!empty()) pop();
    }

    bool empty() const
    {
        return head == 0;
    }

    void push(Item x)
    {
        head = new Node(x, head);
        cnt++;
    }

    Item pop()
    {
        Item v = head->item;
        Node *t = head->next;

        delete head;
        head = t;
        --cnt;
        return v;
    }

    void add(Item *a, int size)
    {
        for (int i = 0; i < size; i++)
        {
            Item t = a[i];
            push(t);
        }
    }

    int count() const
    {
        return cnt;
    }

private:
    struct Node
    {
        Item item;
        Node *next;

        Node(Item x, Node* t)
        {
            item = x;
            next = t;
        }
    };

    Node *head;
    int cnt;
};


template <class Item>
class QUEUE
{
public:
    QUEUE()
    {
        head = 0;
        cnt = 0;
        max = 0;
    }

    QUEUE(int size)
    {
        head = 0;
        cnt = 0;
        max = size;
    }

    ~QUEUE()
    {
        remove(cnt);
    }

    int empty() const
    {
        return head == 0;
    }

    void put(Item x)
    {
        Node *t = tail;
        tail = new Node(x);
        //tail = (Node*)malloc(sizeof(Node));
        tail->item = x;
        tail->next = 0;

        if (head == 0)
        {
            head = tail;            
        }
        else t->next = tail;        

        cnt++;

        if (cnt > max && max > 0) get();
    }


    Item get()
    {
        Item v = head->item;
        Node *t = head->next;

        delete head;
        //free(head);
        head = t;
        --cnt;

        return v;
    }

    Item at(int n)
    {
        if (n >= count()) n = count() - 1;

        Item v = head->item;
        if (n == 0) return v;

        Node *index = head->next;
        for (int i = 1; i <= n; i++)
        {
            v = index->item;
            index = index->next;
        }

        return v;
    }

	Item last()
	{
		Item v = tail->item;

		return v;
	}

	Item first()
	{
		Item v = head->item;

		return v;
	}

    void add(Item *a, int size)
    {
        for (int i = 0; i < size; i++)
        {
            Item t = a[i];
            put(t);
        }
    }

    /*void add(QUEUE<Item> *queue, int num)
    {
        int size = queue->count();
        for (int i = num; i < size; i++)
        {
            Item t = queue->at(i);
            put(t);
        }
    }*/

    int count() const
    {
        return cnt;
    }

    int max_size()
    {
        return max;
    }

    void set_max_size(int size)
    {
        if (size > 0) max = size;
    }

    void remove(int size)
    {
        if (size > cnt) size = cnt;
        //Item v;

        for (int i = 0; i < size; i++)
        {
            get();
        }
    }

    void clear()
    {
        int size = cnt;
        /*for (int i = 0; i < size; i++)
        {
            get();
        }*/
        remove(size);
    }

	QUEUE<Item> &QUEUE<Item>::operator = (const QUEUE<Item> &queue)
	{
		clear();
		int size = queue.count();
		max = queue.max_size();
		for (int i = 0; i < size; i++) 
		{
			Item item = queue.get();
			put(item);
		}

		return this;
	}

private:
    struct Node
    {        
        Item item;
        Node *next;

        Node(Item x)
        {
            item = x;
            next = 0;
        }  		
    };

    Node *head, *tail;
    int max;
    int cnt;
};



/*template <class Item>
class QUEUE_PTR
{
public:
    QUEUE_PTR()
    {
        head = 0;
        cnt = 0;
        max = 0;
    }

    QUEUE_PTR(int size)
    {
        head = 0;
        cnt = 0;
        max = size;
    }

    ~QUEUE_PTR()
    {
        remove(cnt);
    }
    
    void put(Item *x)
    {
        Node **t = tail;
        tail = new Node(x);        
        tail->item = x;
        tail->next = 0;
		tail->prev = t;

        if (head == 0)
        {
            head = tail;            
        }
        else t->next = tail;        

        cnt++;

        if (cnt > max && max > 0) get();
    }

	Item* at(int n)
	{
		if (n >= count()) n = count() - 1;

		Item *v = head->item;
		if (n == 0) return v;

		Node **index = head->next;
		for (int i = 1; i <= n; i++)
		{
			v = index->item;
			index = index->next;
		}

		return v;
	}

    Item* get()
    {
        Item *v = head->item;
        Node **t = head->next;		

        delete head;
		t->prev = 0;
        head = t;
        --cnt;

        return v;
    }

	Item* getAt(int n)
	{
		if (count() == 0) return 0;

		if (n >= count()) n = count() - 1;		
		if (n == 0) return get();

		Item *v = head->item;
		Node **index = head->next;
		for (int i = 1; i < n; i++)
		{
			v = index->item;
			index = index->next;				
		}

		Node **t2 = index;
		index = index->prev;
		Node **t1 = index->prev;
		--cnt;
		
		delete index;
		t1->next = t2;
		t2->prev = t1;

		return v;
	}
        
    void remove(int size)
    {
        if (size > cnt) size = cnt;
        
        for (int i = 0; i < size; i++)
        {
            Item *v = get();
			delete v;
        }
    }

    void clear()
    {
        int size = cnt;
        remove(size);
    }

	void set_max_size(int size)
	{
		if (size > 0) max = size;

		int sz = count();
		if (sz < max)
		{
			remove(max-sz);
		}		
	}

	int count() const
	{
		return cnt;
	}

	int max_size()
	{
		return max;
	}

	
private:
    struct Node
    {        
        Item *item;
        Node **next;
		Node **prev;

        Node(Item *x)
        {
            item = x;
            next = 0;
			prev = 0;
        }  		
    };

    Node **head, **tail;
    int max;
    int cnt;
};*/


// Ring buffer for fast data receiving and decoding
#define RBUF_SIZE		512

class RING_BUFFER
{
public:
	RING_BUFFER(int _max_len = RBUF_SIZE)
	{
		init(_max_len);
	}

	// Initialize the ring buffer with _max_len
	void init(int _max_len)
	{
		max_len = _max_len;		
		flush();
	}

	// Flush the ring buffer
	void flush()
	{
		memset(&data[0], 0x0, RBUF_SIZE*sizeof(uint8_t));
		tail = 0;
		head = 0;
		cnt = 0;
	}

	// Clears the ring buffer 
	// But doesn't fill in the buffer by zeros !
	void clear()
	{
		tail = 0;
		head = 0;
		cnt = 0;
	}

	// Check if the ring buffer is empty
	bool empty()
	{
		return cnt == 0;
	}

	// Check if the ring buffer is full
	bool full()
	{
		return cnt == max_len;
	}

	// Returns the number of bytes in the ring buffer
	int count()
	{
		return cnt;
	}

	// Put a byte to the ring buffer.
	// Returns false if the ring buffer is full
	void put(uint8_t v, bool *ok = 0)
	{
		data[tail] = v;	

		bool is_full = full();
		if (!is_full)
		{			
			cnt++;
			if (tail < max_len-1) tail++;
			else tail = 0;	
			*ok = 1;
		}
		else 
		{
			if (tail < max_len-1) tail++;
			else tail = 0;
			if (head < max_len-1) head++;
			else head = 0;	
			*ok = 0;
		}			
	}

	// Put N bytes to the ring buffer.
	// Returns false if the ring buffer has not enough space
	void put_bytes(uint8_t *_data, int N, bool *ok = 0)
	{		
		for (int i = 0; i < N; i++)
		{
			uint8_t v = _data[i];
			put(v, ok);
		}
	}
		
	// Returns first byte from the head of the ring buffer.
	// If the buffer is empty then returns zero (check buffer size before using!)
	uint8_t get(bool *ok = 0)
	{
		if (cnt == 0) 
		{
			*ok = 0;
			return 0;
		}

		uint8_t v = data[head];
		if (head == max_len-1) head = 0;
		else head++;
		--cnt;

		*ok = 1;
		return v;
	}

	// Returns last byte entered to the ring buffer.
	// If the buffer is empty then returns zero (check buffer size before using!)
	uint8_t get_last(bool *ok = 0)
	{
		if (cnt == 0) 
		{
			*ok = 0;
			return 0;
		}

		int index = tail-1;
		if (index < 0) index = max_len-1;

		uint8_t v = data[index];
		tail = index;
		--cnt;

		*ok = 1;
		return v;
	}

	// Returns all data from the ring buffer
	// Array _data must be initialized before using !
	void get_all(uint8_t *_data, bool *ok = 0)
	{
		if (cnt == 0) 
		{
			*ok = 0;			
			return;
		}

		if (tail > head)
		{
			memcpy(_data, &data[head], cnt*sizeof(uint8_t));
		}
		else
		{
			int len1 = max_len - head;
			memcpy(_data, &data[head], len1*sizeof(uint8_t));
			int len2 = cnt - len1;
			memcpy(_data+len1, &data[0], len2*sizeof(uint8_t));
		}

		*ok = 1;
		clear();
	}

	// Returns N data bytes from the ring buffer
	// Array _data must be initialized before using !
	void get_bytes(uint8_t *_data, int N, bool *ok = 0)
	{
		if (cnt == 0 || cnt < N)
		{
			*ok = 0;
			return;
		}

		for (int i = 0; i < N; i++)
		{
			_data[i] = get(ok);
		}
	}

	// Returns an element number index (head has number 0)
	// If index > cnt then returns 0 and *ok = 0
	uint8_t at(int pos, bool *ok = 0)
	{
		if (pos > cnt-1) 
		{
			*ok = 0;
			return 0;
		}

		int index = head + pos;
		if (index >= max_len) index -= max_len;	

		return data[index];
	}

	// Returns first element in the ring buffer
	uint8_t first()
	{
		return data[head];
	}

	// Returns last element in the ring buffer
	uint8_t last()
	{
		int index = tail-1;
		if (index < 0) index = max_len-1;

		return data[index];
	}

private:
	uint8_t data[RBUF_SIZE];	// ring data store
	int head;
	int tail;
	int cnt;
	int max_len;
};


#endif // IO_CONTAINERS_H
