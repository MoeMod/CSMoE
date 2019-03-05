//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linked list container class 
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#ifndef UTLLINKEDLIST_H
#define UTLLINKEDLIST_H

#ifdef _WIN32
#pragma once
#endif

#include "UtlMemory.h"

// define to enable asserts griping about things you shouldn't be doing with multilists
// #define MULTILIST_PEDANTIC_ASSERTS 1 

// This is a useful macro to iterate from head to tail in a linked list.
#define FOR_EACH_LL( listName, iteratorName ) \
	for( int iteratorName=(listName).Head(); (listName).IsUtlLinkedList && iteratorName != (listName).InvalidIndex(); iteratorName = (listName).Next( iteratorName ) )

//-----------------------------------------------------------------------------
// class CUtlLinkedList:
// description:
//		A lovely index-based linked list! T is the class type, I is the index
//		type, which usually should be an unsigned short or smaller. However,
//		you must avoid using 16- or 8-bit arithmetic on PowerPC architectures; 
//		therefore you should not use UtlLinkedListElem_t::I as the type of 
//		a local variable... ever. PowerPC integer arithmetic must be 32- or 
//		64-bit only; otherwise performance plummets.
//-----------------------------------------------------------------------------

template <class T, class I> 
struct UtlLinkedListElem_t
{
	T  m_Element;
	I  m_Previous;
	I  m_Next;

private:
	// No copy constructor for these...
	UtlLinkedListElem_t( const UtlLinkedListElem_t& );
};


// Class S is the storage type; the type you can use to save off indices in 
// persistent memory. Class I is the iterator type, which is what should be used
// in local scopes. I defaults to be S, but be aware that on the 360, 16-bit 
// arithmetic is catastrophically slow. Therefore you should try to save shorts
// in memory, but always operate on 32's or 64's in local scope.
// The ideal parameter order would be TSMI (you are more likely to override M than I)
// but since M depends on I we can't have the defaults in that order, alas.
template <class T, class S = unsigned short, bool ML = false, class I = S, class M = CUtlMemory< UtlLinkedListElem_t<T, S>, I > > 
class CUtlLinkedList
{
public:
	typedef T ElemType_t;
	typedef S IndexType_t; // should really be called IndexStorageType_t, but that would be a huge change
	typedef I IndexLocalType_t;
	typedef M MemoryAllocator_t;
	static const bool IsUtlLinkedList = true; // Used to match this at compiletime 

	// constructor, destructor
	CUtlLinkedList( int growSize = 0, int initSize = 0 );
	~CUtlLinkedList();

	// gets particular elements
	T&         Element( I i );
	T const&   Element( I i ) const;
	T&         operator[]( I i );
	T const&   operator[]( I i ) const;

	// Make sure we have a particular amount of memory
	void EnsureCapacity( int num );

	void SetGrowSize( int growSize );

	// Memory deallocation
	void Purge();

	// Delete all the elements then call Purge.
	void PurgeAndDeleteElements();
	
	// Insertion methods....
	I	InsertBefore( I before );
	I	InsertAfter( I after );
	I	AddToHead( ); 
	I	AddToTail( );

	I	InsertBefore( I before, T const& src );
	I	InsertAfter( I after, T const& src );
	I	AddToHead( T const& src ); 
	I	AddToTail( T const& src );

	// Find an element and return its index or InvalidIndex() if it couldn't be found.
	I		Find( const T &src ) const;
	
	// Look for the element. If it exists, remove it and return true. Otherwise, return false.
	bool	FindAndRemove( const T &src );

	// Removal methods
	void	Remove( I elem );
	void	RemoveAll();

	// Allocation/deallocation methods
	// If multilist == true, then list list may contain many
	// non-connected lists, and IsInList and Head + Tail are meaningless...
	I		Alloc( bool multilist = false );
	void	Free( I elem );

	// list modification
	void	LinkBefore( I before, I elem );
	void	LinkAfter( I after, I elem );
	void	Unlink( I elem );
	void	LinkToHead( I elem );
	void	LinkToTail( I elem );

	// invalid index (M will never allocate an element at this index)
	inline static S  InvalidIndex()  { return ( S )M::InvalidIndex(); }

	// Is a given index valid to use? (representible by S and not the invalid index)
	static bool IndexInRange( I index );

	inline static size_t ElementSize() { return sizeof( ListElem_t ); }

	// list statistics
	int	Count() const;
	I	MaxElementIndex() const;
	I	NumAllocated( void ) const { return m_NumAlloced; }

	// Traversing the list
	I  Head() const;
	I  Tail() const;
	I  Previous( I i ) const;
	I  Next( I i ) const;

	// STL compatible const_iterator class
	template < typename List_t >
	class _CUtlLinkedList_constiterator_t
	{
	public:
		typedef typename List_t::ElemType_t ElemType_t;
		typedef typename List_t::IndexType_t IndexType_t;

		// Default constructor -- gives a currently unusable iterator.
		_CUtlLinkedList_constiterator_t()
			: m_list( 0 )
			, m_index( List_t::InvalidIndex() )
		{
		}
		// Normal constructor.
		_CUtlLinkedList_constiterator_t( const List_t& list, IndexType_t index )
			: m_list( &list )
			, m_index( index )
		{
		}

		// Pre-increment operator++. This is the most efficient increment
		// operator so it should always be used.
		_CUtlLinkedList_constiterator_t& operator++()
		{
			m_index = m_list->Next( m_index );
			return *this;
		}
		// Post-increment operator++. This is less efficient than pre-increment.
		_CUtlLinkedList_constiterator_t operator++(int)
		{
			// Copy ourselves.
			_CUtlLinkedList_constiterator_t temp = *this;
			// Increment ourselves.
			++*this;
			// Return the copy.
			return temp;
		}

		// Pre-decrement operator--. This is the most efficient decrement
		// operator so it should always be used.
		_CUtlLinkedList_constiterator_t& operator--()
		{
			Assert( m_index != m_list->Head() );
			if ( m_index == m_list->InvalidIndex() )
			{
				m_index = m_list->Tail();
			}
			else
			{
				m_index = m_list->Previous( m_index );
			}
			return *this;
		}
		// Post-decrement operator--. This is less efficient than post-decrement.
		_CUtlLinkedList_constiterator_t operator--(int)
		{
			// Copy ourselves.
			_CUtlLinkedList_constiterator_t temp = *this;
			// Decrement ourselves.
			--*this;
			// Return the copy.
			return temp;
		}

		bool operator==( const _CUtlLinkedList_constiterator_t& other) const
		{
			Assert( m_list == other.m_list );
			return m_index == other.m_index;
		}

		bool operator!=( const _CUtlLinkedList_constiterator_t& other) const
		{
			Assert( m_list == other.m_list );
			return m_index != other.m_index;
		}

		const ElemType_t& operator*() const
		{
			return m_list->Element( m_index );
		}

		const ElemType_t* operator->() const
		{
			return (&**this);
		}

	protected:
		// Use a pointer rather than a reference so that we can support
		// assignment of iterators.
		const List_t* m_list;
		IndexType_t m_index;
	};

	// Are nodes in the list or valid?
	bool  IsValidIndex( I i ) const;
	bool  IsInList( I i ) const;
   
protected:

	// What the linked list element looks like
	typedef UtlLinkedListElem_t<T, S>  ListElem_t;

	// constructs the class
	I		AllocInternal( bool multilist = false );
	void ConstructList();
	
	// Gets at the list element....
	ListElem_t& InternalElement( I i ) { return m_Memory[i]; }
	ListElem_t const& InternalElement( I i ) const { return m_Memory[i]; }

	// copy constructors not allowed
	CUtlLinkedList( CUtlLinkedList<T, S, ML, I, M> const& list ) { Assert(0); }

	M	m_Memory;
	I	m_Head;
	I	m_Tail;
	I	m_FirstFree;
	I	m_ElementCount;		// The number actually in the list
	I	m_NumAlloced;		// The number of allocated elements
	typename M::Iterator_t	m_LastAlloc; // the last index allocated

	// For debugging purposes; 
	// it's in release builds so this can be used in libraries correctly
	ListElem_t  *m_pElements;

	inline M const &Memory( void ) const
	{
		return m_Memory;
	}

	void ResetDbgInfo()
	{
		m_pElements = m_Memory.Base();
	}

private:
	// Faster version of Next that can only be used from tested code internal
	// to this class, such as Find(). It avoids the cost of checking the index
	// validity, which is a big win on debug builds.
	I  PrivateNext( I i ) const;
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
CUtlLinkedList<T,S,ML,I,M>::CUtlLinkedList( int growSize, int initSize ) :
	m_Memory( growSize, initSize ), m_LastAlloc( m_Memory.InvalidIterator() )
{
	// Prevent signed non-int datatypes
	ConstructList();
	ResetDbgInfo();
}

template <class T, class S, bool ML, class I, class M>
CUtlLinkedList<T,S,ML,I,M>::~CUtlLinkedList( ) 
{
	RemoveAll();
}

template <class T, class S, bool ML, class I, class M>
void CUtlLinkedList<T,S,ML,I,M>::ConstructList()
{
	m_Head = InvalidIndex(); 
	m_Tail = InvalidIndex();
	m_FirstFree = InvalidIndex();
	m_ElementCount = 0;
	m_NumAlloced = 0;
}


//-----------------------------------------------------------------------------
// gets particular elements
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
inline T& CUtlLinkedList<T,S,ML,I,M>::Element( I i )
{
	return m_Memory[i].m_Element; 
}

template <class T, class S, bool ML, class I, class M>
inline T const& CUtlLinkedList<T,S,ML,I,M>::Element( I i ) const
{
	return m_Memory[i].m_Element; 
}

template <class T, class S, bool ML, class I, class M>
inline T& CUtlLinkedList<T,S,ML,I,M>::operator[]( I i )        
{ 
	return m_Memory[i].m_Element; 
}

template <class T, class S, bool ML, class I, class M>
inline T const& CUtlLinkedList<T,S,ML,I,M>::operator[]( I i ) const 
{
	return m_Memory[i].m_Element; 
}

//-----------------------------------------------------------------------------
// list statistics
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
inline int CUtlLinkedList<T,S,ML,I,M>::Count() const      
{ 
#ifdef MULTILIST_PEDANTIC_ASSERTS
	AssertMsg( !ML, "CUtlLinkedList::Count() is meaningless for linked lists." );
#endif
	return m_ElementCount; 
}

template <class T, class S, bool ML, class I, class M>
inline I CUtlLinkedList<T,S,ML,I,M>::MaxElementIndex() const   
{
	return m_Memory.NumAllocated();
}


//-----------------------------------------------------------------------------
// Traversing the list
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
inline I  CUtlLinkedList<T,S,ML,I,M>::Head() const  
{ 
	return m_Head; 
}

template <class T, class S, bool ML, class I, class M>
inline I  CUtlLinkedList<T,S,ML,I,M>::Tail() const  
{ 
	return m_Tail; 
}

template <class T, class S, bool ML, class I, class M>
inline I  CUtlLinkedList<T,S,ML,I,M>::Previous( I i ) const  
{ 
	Assert( IsValidIndex(i) ); 
	return InternalElement(i).m_Previous; 
}

template <class T, class S, bool ML, class I, class M>
inline I  CUtlLinkedList<T,S,ML,I,M>::Next( I i ) const  
{ 
	Assert( IsValidIndex(i) ); 
	return InternalElement(i).m_Next; 
}

template <class T, class S, bool ML, class I, class M>
inline I  CUtlLinkedList<T,S,ML,I,M>::PrivateNext( I i ) const  
{ 
	return InternalElement(i).m_Next; 
}


//-----------------------------------------------------------------------------
// Are nodes in the list or valid?
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable: 4310 ) // Allows "(I)(S)M::INVALID_INDEX" below
#endif
template <class T, class S, bool ML, class I, class M>
inline bool CUtlLinkedList<T,S,ML,I,M>::IndexInRange( I index ) // Static method
{
	// Since S is not necessarily the type returned by M, we need to check that M returns indices
	// which are representable by S. A common case is 'S === unsigned short', 'I == int', in which
	// case CUtlMemory will have 'InvalidIndex == (int)-1' (which casts to 65535 in S), and will
	// happily return elements at index 65535 and above.

	return ( ( (S)index == index ) && ( (S)index != InvalidIndex() ) );
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

template <class T, class S, bool ML, class I, class M>
inline bool CUtlLinkedList<T,S,ML,I,M>::IsValidIndex( I i ) const  
{
	if ( !m_Memory.IsIdxValid( i ) )
		return false;

	if ( m_Memory.IsIdxAfter( i, m_LastAlloc ) )
		return false; // don't read values that have been allocated, but not constructed

	return ( m_Memory[ i ].m_Previous != i ) || ( m_Memory[ i ].m_Next == i );
}

template <class T, class S, bool ML, class I, class M>
inline bool CUtlLinkedList<T,S,ML,I,M>::IsInList( I i ) const
{
	if ( !m_Memory.IsIdxValid( i ) || m_Memory.IsIdxAfter( i, m_LastAlloc ) )
		return false; // don't read values that have been allocated, but not constructed

	return Previous( i ) != i;
}

/*
template <class T>
inline bool CUtlFixedLinkedList<T>::IsInList( int i ) const
{
	return m_Memory.IsIdxValid( i ) && (Previous( i ) != i);
}
*/

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------

template< class T, class S, bool ML, class I, class M >
void CUtlLinkedList<T,S,ML,I,M>::EnsureCapacity( int num )
{
	//MEM_ALLOC_CREDIT_CLASS();
	m_Memory.EnsureCapacity(num);
	ResetDbgInfo();
}

template< class T, class S, bool ML, class I, class M >
void CUtlLinkedList<T,S,ML,I,M>::SetGrowSize( int growSize )
{
	RemoveAll();
	m_Memory.Init( growSize );
	ResetDbgInfo();
}


//-----------------------------------------------------------------------------
// Deallocate memory
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::Purge()
{
	RemoveAll();

	m_Memory.Purge();
	m_FirstFree = InvalidIndex();
	m_NumAlloced = 0;

	//Routing "m_LastAlloc = m_Memory.InvalidIterator();" through a local const to sidestep an internal compiler error on 360 builds
	const typename M::Iterator_t scInvalidIterator = m_Memory.InvalidIterator();
	m_LastAlloc = scInvalidIterator;
	ResetDbgInfo();
}


template<class T, class S, bool ML, class I, class M>
void CUtlLinkedList<T,S,ML,I,M>::PurgeAndDeleteElements()
{
	I iNext;
	for( I i=Head(); i != InvalidIndex(); i=iNext )
	{
		iNext = Next(i);
		delete Element(i);
	}

	Purge();
}


//-----------------------------------------------------------------------------
// Node allocation/deallocation
//-----------------------------------------------------------------------------
template <class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::AllocInternal( bool multilist )
{
	Assert( !multilist || ML );
#ifdef MULTILIST_PEDANTIC_ASSERTS
	Assert( multilist == ML );
#endif
	I elem;
	if ( m_FirstFree == InvalidIndex() )
	{
		Assert( m_Memory.IsValidIterator( m_LastAlloc ) || m_ElementCount == 0 );

		typename M::Iterator_t it = m_Memory.IsValidIterator( m_LastAlloc ) ? m_Memory.Next( m_LastAlloc ) : m_Memory.First();

		if ( !m_Memory.IsValidIterator( it ) )
		{
			m_Memory.Grow();
			ResetDbgInfo();

			it = m_Memory.IsValidIterator( m_LastAlloc ) ? m_Memory.Next( m_LastAlloc ) : m_Memory.First();

			Assert( m_Memory.IsValidIterator( it ) );
			if ( !m_Memory.IsValidIterator( it ) )
			{
				// We rarely if ever handle alloc failure. Continuing leads to corruption.
				//Error( "CUtlLinkedList overflow! (exhausted memory allocator)\n" );
				return InvalidIndex();
			}
		}

		// We can overflow before the utlmemory overflows, since S != I
		if ( !IndexInRange( m_Memory.GetIndex( it ) ) )
		{
			// We rarely if ever handle alloc failure. Continuing leads to corruption.
			//Error( "CUtlLinkedList overflow! (exhausted index range)\n" );
			return InvalidIndex();
		}

		m_LastAlloc = it;
		elem = m_Memory.GetIndex( m_LastAlloc );
		m_NumAlloced++;
	} 
	else
	{
		elem = m_FirstFree;
		m_FirstFree = InternalElement( m_FirstFree ).m_Next;
	}

	if ( !multilist )
	{
		InternalElement( elem ).m_Next = elem;
		InternalElement( elem ).m_Previous = elem;
	}
	else
	{
		InternalElement( elem ).m_Next = InvalidIndex();
		InternalElement( elem ).m_Previous = InvalidIndex();
	}

	return elem;
}

template <class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::Alloc( bool multilist )
{
	I elem = AllocInternal( multilist );
	if ( elem == InvalidIndex() )
		return elem;

	Construct( &Element(elem) );

	return elem;
}

template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::Free( I elem )
{
	Assert( IsValidIndex(elem) && IndexInRange( elem ) );
	Unlink(elem);

	ListElem_t &internalElem = InternalElement(elem);
	Destruct( &internalElem.m_Element );
	internalElem.m_Next = m_FirstFree;
	m_FirstFree = elem;
}

//-----------------------------------------------------------------------------
// Insertion methods; allocates and links (uses default constructor)
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::InsertBefore( I before )
{
	// Make a new node
	I   newNode = AllocInternal();
	if ( newNode == InvalidIndex() )
		return newNode;

	// Link it in
	LinkBefore( before, newNode );
	
	// Construct the data
	Construct( &Element(newNode) );
	
	return newNode;
}

template <class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::InsertAfter( I after )
{
	// Make a new node
	I   newNode = AllocInternal();
	if ( newNode == InvalidIndex() )
		return newNode;

	// Link it in
	LinkAfter( after, newNode );
	
	// Construct the data
	Construct( &Element(newNode) );
	
	return newNode;
}

template <class T, class S, bool ML, class I, class M>
inline I CUtlLinkedList<T,S,ML,I,M>::AddToHead( ) 
{ 
	return InsertAfter( InvalidIndex() ); 
}

template <class T, class S, bool ML, class I, class M>
inline I CUtlLinkedList<T,S,ML,I,M>::AddToTail( ) 
{ 
	return InsertBefore( InvalidIndex() ); 
}


//-----------------------------------------------------------------------------
// Insertion methods; allocates and links (uses copy constructor)
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::InsertBefore( I before, T const& src )
{
	// Make a new node
	I   newNode = AllocInternal();
	if ( newNode == InvalidIndex() )
		return newNode;

	// Link it in
	LinkBefore( before, newNode );
	
	// Construct the data
	CopyConstruct( &Element(newNode), src );
	
	return newNode;
}

template <class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::InsertAfter( I after, T const& src )
{
	// Make a new node
	I   newNode = AllocInternal();
	if ( newNode == InvalidIndex() )
		return newNode;

	// Link it in
	LinkAfter( after, newNode );
	
	// Construct the data
	CopyConstruct( &Element(newNode), src );
	
	return newNode;
}

template <class T, class S, bool ML, class I, class M>
inline I CUtlLinkedList<T,S,ML,I,M>::AddToHead( T const& src ) 
{ 
	return InsertAfter( InvalidIndex(), src ); 
}

template <class T, class S, bool ML, class I, class M>
inline I CUtlLinkedList<T,S,ML,I,M>::AddToTail( T const& src ) 
{ 
	return InsertBefore( InvalidIndex(), src ); 
}


//-----------------------------------------------------------------------------
// Removal methods
//-----------------------------------------------------------------------------

template<class T, class S, bool ML, class I, class M>
I CUtlLinkedList<T,S,ML,I,M>::Find( const T &src ) const
{
	// Cache the invalidIndex to avoid two levels of function calls on each iteration.
	I invalidIndex = InvalidIndex();
	for ( I i=Head(); i != invalidIndex; i = PrivateNext( i ) )
	{
		if ( Element( i ) == src )
			return i;
	}
	return InvalidIndex();
}


template<class T, class S, bool ML, class I, class M>
bool CUtlLinkedList<T,S,ML,I,M>::FindAndRemove( const T &src )
{
	I i = Find( src );
	if ( i == InvalidIndex() )
	{
		return false;
	}
	else
	{
		Remove( i );
		return true;
	}
}


template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::Remove( I elem )
{
	Free( elem );
}

template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::RemoveAll()
{
	// Have to do some convoluted stuff to invoke the destructor on all
	// valid elements for the multilist case (since we don't have all elements
	// connected to each other in a list).

	if ( m_LastAlloc == m_Memory.InvalidIterator() )
	{
		Assert( m_Head == InvalidIndex() );
		Assert( m_Tail == InvalidIndex() );
		Assert( m_FirstFree == InvalidIndex() );
		Assert( m_ElementCount == 0 );
		return;
	}

	if ( ML )
	{
		for ( typename M::Iterator_t it = m_Memory.First(); it != m_Memory.InvalidIterator(); it = m_Memory.Next( it ) )
		{
			I i = m_Memory.GetIndex( it );
			if ( IsValidIndex( i ) ) // skip elements already in the free list
			{
				ListElem_t &internalElem = InternalElement( i );
				Destruct( &internalElem.m_Element );
				internalElem.m_Previous = i;
				internalElem.m_Next = m_FirstFree;
				m_FirstFree = i;
			}

			if ( it == m_LastAlloc )
				break; // don't destruct elements that haven't ever been constructed
		}
	}
	else
	{
		I i = Head();
		I next;
		while ( i != InvalidIndex() )
		{
			next = Next( i );
			ListElem_t &internalElem = InternalElement( i );
			Destruct( &internalElem.m_Element );
			internalElem.m_Previous = i;
			internalElem.m_Next = next == InvalidIndex() ? m_FirstFree : next;
			i = next;
		}
		if ( Head() != InvalidIndex() )
		{
			m_FirstFree = Head();
		}
	}

	// Clear everything else out
	m_Head = InvalidIndex(); 
	m_Tail = InvalidIndex();
	m_ElementCount = 0;
}


//-----------------------------------------------------------------------------
// list modification
//-----------------------------------------------------------------------------

template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::LinkBefore( I before, I elem )
{
	Assert( IsValidIndex(elem) );
	
	// Unlink it if it's in the list at the moment
	Unlink(elem);
	
	ListElem_t *pNewElem = &InternalElement(elem);
	
	// The element *after* our newly linked one is the one we linked before.
	pNewElem->m_Next = before;
	
	S newElem_mPrevious; // we need to hang on to this for the compairson against InvalidIndex()
					// below; otherwise we get a a load-hit-store on pNewElem->m_Previous, even
					// with RESTRICT
	if (before == InvalidIndex())
	{
		// In this case, we're linking to the end of the list, so reset the tail
		newElem_mPrevious = m_Tail;
		pNewElem->m_Previous = m_Tail;
		m_Tail = elem;
	}
	else
	{
		// Here, we're not linking to the end. Set the prev pointer to point to
		// the element we're linking.
		Assert( IsInList(before) );
		ListElem_t *beforeElem = &InternalElement(before);
		pNewElem->m_Previous = newElem_mPrevious = beforeElem->m_Previous;
		beforeElem->m_Previous = elem;
	}
	
	// Reset the head if we linked to the head of the list
	if (newElem_mPrevious == InvalidIndex())
		m_Head = elem;
	else
		InternalElement(newElem_mPrevious).m_Next = elem;
	
	// one more element baby
	++m_ElementCount;
}

template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::LinkAfter( I after, I elem )
{
	Assert( IsValidIndex(elem) );
	
	// Unlink it if it's in the list at the moment
	if ( IsInList(elem) )
		Unlink(elem);
	
	ListElem_t& newElem = InternalElement(elem);
	
	// The element *before* our newly linked one is the one we linked after
	newElem.m_Previous = after;
	if (after == InvalidIndex())
	{
		// In this case, we're linking to the head of the list, reset the head
		newElem.m_Next = m_Head;
		m_Head = elem;
	}
	else
	{
		// Here, we're not linking to the end. Set the next pointer to point to
		// the element we're linking.
		Assert( IsInList(after) );
		ListElem_t& afterElem = InternalElement(after);
		newElem.m_Next = afterElem.m_Next;
		afterElem.m_Next = elem;
	}
	
	// Reset the tail if we linked to the tail of the list
	if (newElem.m_Next == InvalidIndex())
		m_Tail = elem;
	else
		InternalElement(newElem.m_Next).m_Previous = elem;
	
	// one more element baby
	++m_ElementCount;
}

template <class T, class S, bool ML, class I, class M>
void  CUtlLinkedList<T,S,ML,I,M>::Unlink( I elem )
{
	Assert( IsValidIndex(elem) );
	if (IsInList(elem))
	{
		ListElem_t *pOldElem = &m_Memory[ elem ];

		// If we're the first guy, reset the head
		// otherwise, make our previous node's next pointer = our next
		if ( pOldElem->m_Previous != InvalidIndex() )
		{
			m_Memory[ pOldElem->m_Previous ].m_Next = pOldElem->m_Next;
		}
		else
		{
			m_Head = pOldElem->m_Next;
		}

		// If we're the last guy, reset the tail
		// otherwise, make our next node's prev pointer = our prev
		if ( pOldElem->m_Next != InvalidIndex() )
		{
			m_Memory[ pOldElem->m_Next ].m_Previous = pOldElem->m_Previous;
		}
		else
		{
			m_Tail = pOldElem->m_Previous;
		}

		// This marks this node as not in the list, 
		// but not in the free list either
		pOldElem->m_Previous = pOldElem->m_Next = elem;

		// One less puppy
		--m_ElementCount;
	}
}

template <class T, class S, bool ML, class I, class M>
inline void CUtlLinkedList<T,S,ML,I,M>::LinkToHead( I elem ) 
{
	LinkAfter( InvalidIndex(), elem ); 
}

template <class T, class S, bool ML, class I, class M>
inline void CUtlLinkedList<T,S,ML,I,M>::LinkToTail( I elem ) 
{
	LinkBefore( InvalidIndex(), elem ); 
}


//-----------------------------------------------------------------------------

#endif // UTLLINKEDLIST_H
