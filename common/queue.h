/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
	------------------------------------------------------------------------------------
	This program is free software; you can redistribute it and/or modify it under
	the terms of the GNU Lesser General Public License as published by the Free Software
	Foundation; either version 2 of the License, or (at your option) any later
	version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License along with
	this program; if not, write to the Free Software Foundation, Inc., 59 Temple
	Place - Suite 330, Boston, MA 02111-1307, USA, or go to
	http://www.gnu.org/copyleft/lesser.txt.
	------------------------------------------------------------------------------------
	Author:		Zhur
*/

/*
 * this file really needs to die... it is a terrible class. But I havent
 * had time to rewrite the TCP stuff to stop using it.
 *
 *
 *
*/

#ifndef QUEUE_H
#define QUEUE_H

template<class T>
class MyQueue;

template<class T>
class MyQueueNode
{
public:
    MyQueueNode(T* _data)
    {
        next = 0;
        data = _data;
    }

    friend class MyQueue<T>;

private:
    T* data;
    MyQueueNode<T>* next;
};

template<class T>
class MyQueue
{
public:
    MyQueue()
    {
        head = tail = 0;
    }
	~MyQueue() {
		clear();
	}

    void push(T* data)
    {
        if (head == 0)
        {
            tail = head = new MyQueueNode<T>(data);
        }
        else
        {
            tail->next = new MyQueueNode<T>(data);
            tail = tail->next;
        }
    }

    T* pop()
    {
        if (head == 0)
        {
            return 0;
        }

        T* data = head->data;
        MyQueueNode<T>* next_node = head->next;
        delete head;
        head = next_node;

        return data;
    }

    T* top()
    {
        if (head == 0)
        {
            return 0;
        }

        return head->data;
    }

    bool empty()
    {
        if (head == 0)
        {
            return true;
        }

        return false;
    }

    void clear()
    {
		T* d = 0;
        while((d = pop())) {
			delete d;
		}
        return;
    }

    int count()
    {
    	int _count = 0;
		MyQueueNode<T>* d = head;
        while(d != 0) {
        	_count++;
        	d = d->next;
		}
        return(_count);
    }

private:
    MyQueueNode<T>* head;
    MyQueueNode<T>* tail;
};

#endif
