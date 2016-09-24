#include <stdio.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct charFreq 	// the simple structure that holds frequency 'freq' of a character 'ch'
{
	char ch;
	unsigned long long int freq;
};

/**
*	Sorting algorithm
* 	As name suggests, sorts elements in  descending order
* 	MUST BE IMPROVED INTO nLOGn time complexity sort
*/
int compFunc(const void *a,const void *b)
{
	struct charFreq* A  = (struct charFreq*)a;
	struct charFreq* B  = (struct charFreq*)b;
	return ( B->freq - A->freq );
}

void charFreq_descending_sort(struct charFreq *array, const int N)
{
	qsort(array, N, sizeof(struct charFreq) , compFunc);
}

/**
*	NOT INVOKED IN FINAL BUILD OF PROGRAM
* 	Simple print function - Testing purposes
* 	runs until end of array of struct charFreq freq_array
* 	Each value's character, character value and frequency is printed
*/
void charFreq_print(struct charFreq *freq_array, const int N)
{
	printf("\n__Start of Printing Array__\n");
	// for testing purposes
	int i;
	for(i = 0; i < N; ++i)	// runs until N
	{
		// prints values
		printf("%c -> %d -> %llu\n", freq_array[i].ch, (int)freq_array[i].ch, freq_array[i].freq);
	}
	printf("__End of Printing Array__\n\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct leaf
{
	char ch;			// character whos frequency has been mapped
	struct leaf *left;	// self-referential left pointer
	struct leaf *right;	// self referential right pointer
};

struct leaf* create_leaf(char ch, struct leaf* left, struct leaf* right)
{
	struct leaf* myLeaf = malloc(sizeof(struct leaf));	// creates leaf that will be populated and returned
	if(myLeaf==NULL)
	{
		fprintf(stderr, "Failed to allocate memory.\n");
		return NULL;
	}
	myLeaf->ch = ch;		// values populated based on parameters passed
	myLeaf->left = left;	// parameters assumed to be valid
	myLeaf->right = right;
	return myLeaf;			// myLeaf returned
}

void print_leaf_inorder(struct leaf* root, int depth)
{
	if(root == NULL)	// if it is NULL, the return
		return;
	if(root->left == NULL && root->right == NULL)
		printf("_%c(%d)_", root->ch, depth);		// print current node
	else
	{
		print_leaf_inorder(root->left, depth+1);	// print left node
		print_leaf_inorder(root->right, depth+1);// print right node
	}
	
}

void free_leaf(struct leaf* root)	// recursive function to free a tree
{
	if(root==NULL)					// base case that returns when root is NULL
		return;
	free_leaf(root->left);			// otherwise first free left sub-tree
	free_leaf(root->right);			// then free the right sub-tree
	free(root);						// finally free itself <MOST IMPORTANT>
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct node
{
	unsigned long long int freq;	// llu to store frequency of the ROOT it points to
	struct node *next;				// self-referential next pointer to the next link
	struct leaf *root;				// pointer to the root of the tree it represents
};

struct node* create_node(unsigned long long int freq, struct leaf *root)
{
	struct node* myNode = malloc(sizeof(struct node));		// memory allocated for the node
	if(myNode == NULL)		// check if allocation was successful
	{
		fprintf(stderr, "Failed to allocate memory.\n");
		return NULL;
	}
	myNode->freq = freq;		// populate with values that came in as parameters
	myNode->root = root;
	myNode->next = NULL;		// make next link initialized to NULL
	return myNode;				// node returned
}

// for unhuff
struct node* Push_stack(struct node* head, struct leaf* root)
{
	// name very suggestive of functionality
	struct node* new_head = create_node(0, root);	// creates new node based on root provided
	new_head->next = head;	// sets next value to the head
	head = new_head;		// although not needed, shift head to point to new_head
	return head;			// return head. // could alternatively return new_head
}

// for unhuff
struct node* combine_outer_two(struct node* head)
{
	// function to pop two values, merge their roots and insert back into head
	if(head==NULL || head->next == NULL)	// returns head if 2 values are not present for merging
		return head;
	struct node* first = head;		// saves pointer to first struct node in 'first'
	head = head->next;
	struct node* second = head;		// and second pointer to 'second'
	head = head->next;
	struct leaf* root = create_leaf('#', second->root, first->root);	// a new merged root is created
								//   ^ this hashtag(#) being written is of no value and should never need to be read
	free(first);				// free the node first points to
	free(second);				// free the node second points to
	head = Push_stack(head, root);	// push out new leaf into the stack by calling Push_stack
	return head;				// head is returned
}

struct node* insert_into_Stack(struct node* head, struct node* naya)	// function to insert by recursion
{
	if(head == NULL)	// if head is NULL (ie. reached end or empty stack) 
	{					// then return the new struct node
		return naya;
	}
	if(head->freq > naya->freq)	// if we reach a location where insertion is suitable
	{							// make new node point to the current node and return the new one
		naya->next = head;
		return naya;
	}
	head->next = insert_into_Stack(head->next, naya);	// otherwise recursively call yourself at next location
	return head;				// finally return the current node up a level
}

struct node* create_huffTreeStack(struct charFreq *charFreq_array, short sizeof_charFreq_array)
{
	struct node* huffTreeStack = NULL;			//pointer to the head of the stack
	short i = 0;
	for(i = 0; i < sizeof_charFreq_array; ++i)	// loop runs through charFreq_arrray
	{
		// temporary node created that holds the new node we will allocate
		// freq passed as first parameter and a newly created struct leaf*
		// is passed as second parameter with payload of 'ch' and links NULL and NULL
		struct node* temp = create_node( charFreq_array[i].freq , create_leaf(charFreq_array[i].ch, NULL, NULL) );
		temp->next = huffTreeStack;	// temp is made to point to the top of the stack
		huffTreeStack = temp;		// stack top pointer is made to point back to top
	}
	return huffTreeStack;			// list top pointer is returned
}

void print_huffTreeStack(struct node* huffTreeStack)
{
	printf("\n__Start printing treeStack__\n");
	if(huffTreeStack == NULL)	// return if nothing is there to print
	{
		printf(" => Tree Stack is  empty.\n");
		printf("__Finish printing treeStack__\n\n");
		return;
	}
	struct node* ptr_hTS;
	while(huffTreeStack != NULL)	// traverse until end of the stack
	{
		ptr_hTS = huffTreeStack->next;
		printf("%llu: ", huffTreeStack->freq);		// print frequency
		print_leaf_inorder(huffTreeStack->root,0);	// print infix value
		printf("\n");
		huffTreeStack = ptr_hTS;
	}
	printf("__Finish printing treeStack__\n\n");
}

void free_huffTreeStack(struct node* huffTreeStack)
{
	if(huffTreeStack == NULL)	// nothing to free in this case
		return;
	free_huffTreeStack(huffTreeStack->next);
	free_leaf(huffTreeStack->root);	// the leaf attached is first freed
	free(huffTreeStack);			// the current node is freed
}

struct leaf* generate_huffTree(struct node* huffTreeStack)
{
	if(huffTreeStack==NULL)	// if no value was provided
	{
		fprintf(stderr, "Error: cannot generate huffTree out of nothing.\n");
		return NULL;
	}
	struct node* root = huffTreeStack;
	while(huffTreeStack->next != NULL)	// until only one element exists in the the Stack
	{
		struct node* onnu = huffTreeStack;
		huffTreeStack = huffTreeStack->next;	// Over this segment of code we have popped off two values
		struct node* rendu = huffTreeStack;		// from huffTreeStack and saved it in onnu and rendu
//		huffTreeStack = huffTreeStack->next;
		struct node* combo = create_node(onnu->freq + rendu->freq, create_leaf('#', onnu->root, rendu->root));
		//onnu->next = NULL; 
		onnu->root = NULL;	// NOTE. freed because heap memory used in tree
		//free(onnu);
		//onnu = NULL;
		//rendu->next = NULL; 
		rendu->root = NULL;
		//free(rendu);
		//rendu = NULL;
		
		// BELOW need to insert combo back into huffTreeStack such that it is in position
		huffTreeStack = insert_into_Stack(huffTreeStack, combo);

		huffTreeStack = huffTreeStack->next;	// move huffTreeStack pointer after adding node as it can be added there too
											// removed memory error that was bothering for a long time
										// don't know how it removed error though

		// print_huffTreeStack(huffTreeStack);						// testing purposes	TESTING ONLY
	}
	struct leaf* temp = huffTreeStack->root;
	huffTreeStack->root = NULL;
	free_huffTreeStack(root);
	return temp;
}
