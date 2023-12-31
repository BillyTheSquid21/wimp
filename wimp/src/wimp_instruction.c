#include <wimp_instruction.h>

WimpInstrQueue wimp_create_instr_queue()
{
	WimpInstrQueue q;
	q.backnode = NULL;
	q.nextnode = NULL;
	q._queuemutex = p_mutex_new();
	return q;
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

	//Must lock and unlock thread here as adding to a shared space
	p_mutex_lock(queue->_queuemutex);

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

	p_mutex_unlock(queue->_queuemutex);
	printf("Added instr to incoming: %s\n", (char*)new_node->instr.instruction);
	return WIMP_INSTRUCTION_SUCCESS;
}

WimpInstrNode wimp_instr_queue_next(WimpInstrQueue* queue)
{
	p_mutex_lock(queue->_queuemutex);

	//Return null if the queue is exhausted
	if (queue->nextnode == NULL)
	{
		p_mutex_unlock(queue->_queuemutex);
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

	p_mutex_unlock(queue->_queuemutex);
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
	WimpInstrNode currentnode = wimp_instr_queue_next(&queue);
	while (currentnode != NULL)
	{
		wimp_instr_node_free(currentnode);
		currentnode = wimp_instr_queue_next(&queue);
	}

	p_mutex_free(queue._queuemutex);
}