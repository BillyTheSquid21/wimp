#include <wimp_instruction.h>
#include <stdlib.h>

WimpInstrQueue wimp_create_instr_queue()
{
	WimpInstrQueue q;
	q.backnode = NULL;
	q.nextnode = NULL;
	q._datamutex = p_mutex_new();
	q._nextmutex = p_mutex_new();
	q._lowpriomutex = p_mutex_new();
	return q;
}

void wimp_instr_queue_low_prio_lock(WimpInstrQueue* queue)
{
	p_mutex_lock(queue->_lowpriomutex);
	p_mutex_lock(queue->_nextmutex);
	p_mutex_lock(queue->_datamutex);
	p_mutex_unlock(queue->_nextmutex);
}

void wimp_instr_queue_low_prio_unlock(WimpInstrQueue* queue)
{
	p_mutex_unlock(queue->_datamutex);
	p_mutex_unlock(queue->_lowpriomutex);
}

void wimp_instr_queue_high_prio_lock(WimpInstrQueue* queue)
{
	p_mutex_lock(queue->_nextmutex);
	p_mutex_lock(queue->_datamutex);
	p_mutex_unlock(queue->_nextmutex);
}

void wimp_instr_queue_high_prio_unlock(WimpInstrQueue* queue)
{
	p_mutex_unlock(queue->_datamutex);
}

int32_t wimp_instr_queue_add(WimpInstrQueue* queue, void* instr, size_t bytes)
{
	WimpInstrNode new_node = malloc(sizeof(struct _WimpInstrNode));
	if (new_node == NULL)
	{
		return WIMP_INSTRUCTION_FAIL;
	}

	new_node->instr.instruction = instr;
	new_node->instr.instruction_bytes = bytes;
	new_node->nextnode = NULL;

	if (queue->backnode == NULL)
	{
		//There are no nodes in the queue, set both to first node
		queue->nextnode = new_node;
		queue->backnode = new_node;
	}
	else
	{
		//There is a node in the queue, add to its next node and set to back
		queue->backnode->nextnode = new_node;
		queue->backnode = new_node;
	}
	return WIMP_INSTRUCTION_SUCCESS;
}

int32_t wimp_instr_queue_add_existing(WimpInstrQueue* queue, WimpInstrNode node)
{
	node->nextnode = NULL;
	if (queue->backnode == NULL)
	{
		//There are no nodes in the queue, set both to first node
		queue->nextnode = node;
		queue->backnode = node;
	}
	else
	{
		//There is a node in the queue, add to its next node and set to back
		queue->backnode->nextnode = node;
		queue->backnode = node;
	}
	return WIMP_INSTRUCTION_SUCCESS;
}

WimpInstrNode wimp_instr_queue_pop(WimpInstrQueue* queue)
{
	//Return null if the queue is exhausted
	if (queue->nextnode == NULL)
	{
		return NULL;
	}

	//Otherwise, return the node and update the next node
	WimpInstrNode current = queue->nextnode;
	queue->nextnode = current->nextnode;

	if (queue->nextnode == NULL)
	{
		//Ensure won't add to deallocated memory
		queue->backnode = NULL;
	}
	return current;
}

void wimp_instr_node_free(WimpInstrNode node)
{
	free(node->instr.instruction);
	free(node);
}

void wimp_instr_queue_free(WimpInstrQueue queue)
{
	//Iterate any remaining nodes and free their data, and the nodes themselves
	WimpInstrNode currentnode = wimp_instr_queue_pop(&queue);
	while (currentnode != NULL)
	{
		wimp_instr_node_free(currentnode);
		currentnode = wimp_instr_queue_pop(&queue);
	}

	p_mutex_free(queue._datamutex);
	p_mutex_free(queue._nextmutex);
	p_mutex_free(queue._lowpriomutex);
}