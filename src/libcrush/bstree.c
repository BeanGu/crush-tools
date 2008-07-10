/*****************************************
   Copyright 2008 Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *****************************************/

/*
 * implementation of binary search tree functions.  see bstree.h for
 * better function-level documentation.
 */  
  
#include <bstree.h>
  
#ifdef DEBUG
#include <stdio.h>	/* only needed for debug prints */
#endif  /* 
  
/* ****************************** *
 * non-public function prototypes *
 * ****************************** */ 
  
/* this only frees the node itself - not the node data */ 
void bst_destroy_nodes(bst_node_t * node);

/* 2005-07-29 - not implemented yet */ 
void bst_rotate_right(bstree_t * tree, bst_node_t * node);


/* these do all the real work of the *_call_for_each() functions */ 
void bstn_call_preorder(bst_node_t * node, void (*func) (void *));




/* ****************************** *
 * function definitions           *
 * ****************************** */ 
  
/* initialize a binary tree */ 
int bst_init(bstree_t * tree, 
             int (*cmp) (const void *, const void *), 
  
    
  
  
  
  



/* free all of the data in a tree, and then free the nodes */ 
void bst_destroy(bstree_t * tree) {
  
    
  
  



/* free memory  all the nodes in a tree - called from bst_destroy */ 
void bst_destroy_nodes(bst_node_t * node) {
  
    
  
  
  



/* allocate a node, set its data, and make it a child of the specified parent */ 
  bst_node_t * bst_create_child(bstree_t * tree, bst_node_t * parent,
                                void *data) {
  
  
  
    
      /* the parent is specified - won't be the root node */ 
      cmp = tree->cmp(data, parent->data);
    
      
        /* new data greater than parent's data */ 
#ifdef DEBUG
        fprintf(stderr, "parent already has right node.");
      
#endif  /* 
        return NULL;
    
      
        /* new data less than/equal to parent's data */ 
#ifdef DEBUG
        fprintf(stderr, "parent already has left node.");
      
#endif  /* 
        return NULL;
    
  
    
      /* no parent, but the tree already has a root! */ 
#ifdef DEBUG
      fprintf(stderr, "tree already has a root - parent must be specified.");
    
#endif  /* 
      return NULL;
  
  
  
    
  
  
  
  
    
    
      
      
        
      
      else
        
    
      
      
        
      
      else
        
    
    
    /** @bug rotation to rebalance the tree upon node insertion is currently broken */ 
      
      /* 
         
         if ( parent->parent ) {
         if ( parent->balance == leftheavy && parent->parent->balance == leftheavy )
         bst_rotate_left(tree, parent->parent);
         else if ( parent->balance == rightheavy && parent->parent->balance == rightheavy )
         bst_rotate_right(tree, parent->parent);
         }
         
       */ 
  } else {
    
    
  
  



/* insert a node into a tree */ 
  bst_node_t * bst_insert(bstree_t * tree, void *data) {
  
  
    
      
        
      
    
      
        
      
    
  
  



/* delete from a tree the node containing specified data */ 
void bst_delete(bstree_t * tree, void *data) {
  
  
  
    
#ifdef DEBUG
      fprintf(stderr, "node not found\n");
    
#endif  /* 
      return;
  
  
    
      /* leaf node - just delete it and null out the parent pointer */ 
      if (cur->parent) {
      
        
      
      else if (cur->parent->r == cur)
        
    
      
    
  
    
      /* there's a right node, but not a left.  "replace" the
         current node in the tree with its right child */ 
      if (cur->parent->l == cur)
      
    
    else if (cur->parent->r == cur)
      
    
  
    
      /* there's a left node, but not a right.  "replace" the
         current node in the tree with its left child */ 
      if (cur->parent->l == cur)
      
    
    else if (cur->parent->r == cur)
      
    
  
    
    /** @todo add a call to bst_rotate_right() or bst_rotate_left() here after
		  * rotation has been fixed.
		  */ 
      
      /* find the smallest node greater than the current one and put
         it in the place of the current node
       */ 
      bst_node_t * nextgreatest = cur->r;
    
      
    
      /* if this has a right child, push it up to the parent */ 
      if (nextgreatest->r)
      
    
      /* set the pointers */ 
      if (nextgreatest != cur->r)
      
    
    
    
      
        
      
      else
        
    
      
    
  
  
    
  



/* find a node containing the specified data */ 
  bst_node_t * bst_find(bstree_t * tree, void *data) {
  
  
  
    
    
      
    
    else if (cmp < 0)
      
    
    else
      
  
  


  
  
  
    
#ifdef DEBUG
      printf("rotating right-right\n");
    
#endif  /* 
      /* right-right rotation */ 
      node->r = right->l;       /* make the node's right child the one to the left */
    
    
    
      
        
      
      else
        
    
      
    
    
    
    
  
    
    /** @bug in the right-left rotation, grandchild may be NULL, causing a segfault.
		  * likely due to the circumstances in which this type of rotation is performed. */ 
      
#ifdef DEBUG
      printf("rotating right-left\n");
    
#endif  /* 
      /* right-left rotation */ 
      grandchild = right->l;    /* replace left's right child with the grandchild */
    
    
      
    
    
    
    
    
    
      
        
        
        
      
        
        
        
      
        
        
        
    
    
  


  
  
  
    
#ifdef DEBUG
      printf("rotating left-left\n");
    
#endif  /* 
      /* left-left rotation */ 
      node->l = left->r;        /* make the node's left child the one to the right */
    
    
    
      
        
      
      else
        
    
      
    
    
    
    
  
    
    /** @bug in the left-right rotation, grandchild may be NULL, causing a segfault.
		  * likely due to the circumstances in which this type of rotation is performed. */ 
      
#ifdef DEBUG
      printf("rotating left-right\n");
    
#endif  /* 
      /* left-right rotation */ 
      grandchild = left->r;     /* replace left's right child with the grandchild */
    
    
      
    
    
    
    
    
    
      
        
        
        
      
        
        
        
      
        
        
        
    
    
  



/* just a wrapper that pulls out the root node & passes it to the 
   subtree traversal function.
 */ 
void bst_call_for_each(bstree_t * tree, void (*func) (void *),
                       traversal_order_t order) {
  

                             traversal_order_t order) {
  
    
      
      
    
      
      
    
      
      
    
      
      
    
      
  


  
    
  
  
  


  
    
  
  
  


  
    
  
  
  


  
  
  
  
  
    
    
    
      
    
      
  
  


