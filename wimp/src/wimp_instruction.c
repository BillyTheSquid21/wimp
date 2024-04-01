#include <wimp_instruction.h>
#include <stdlib.h>

typedef struct _WimpInstrNode
{
	WimpInstr instr;
	struct _WimpInstrNode* nextnode;
} *WimpInstrNode;

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

WimpInstrMeta wimp_instr_get_from_buffer(uint8_t* buffer, size_t buffsize)
{
	WimpInstrMeta instr;
	instr.arg_bytes = 0;
	instr.dest_process = NULL;
	instr.source_process = NULL;
	instr.instr = NULL;
	instr.args = NULL;
	instr.instr_bytes = 0;
	instr.total_bytes = 0;

	//if buffer is nullptr, was unable to allocate!
	if (buffer == NULL)
	{
		wimp_log_fail("Attempting to get instr from invalid buffer!\n");
		return instr;
	}

	//Size of the full instruction is the start of the buffer - if is \0 is a
	//ping packet so return as is
	if (buffer[0] == '\0')
	{
		return instr;
	}

	instr.dest_process = &buffer[WIMP_INSTRUCTION_DEST_OFFSET];

	//Find start of source process
	char current_char = ' ';
	size_t offset = WIMP_INSTRUCTION_DEST_OFFSET + 1;
	while (current_char != '\0' && offset < buffsize)
	{
		current_char = (char)buffer[offset];
		offset++;
	}

	instr.source_process = &buffer[offset];
	offset++;
	current_char = ' ';

	//Find start of instruction
	while (current_char != '\0' && offset < buffsize)
	{
		current_char = (char)buffer[offset];
		offset++;
	}

	//Record start to get start of instr length
	size_t instr_start = offset;

	instr.instr = &buffer[offset];
	offset++;
	current_char = ' ';

	//Find start of arg bytes
	while (current_char != '\0' && offset < buffsize)
	{
		current_char = (char)buffer[offset];
		offset++;
	}

	//Use diff to get length of instr
	instr.instr_bytes = offset - instr_start;

	instr.arg_bytes = *(int32_t*)&buffer[offset];
	offset += sizeof(int32_t);

	if (instr.arg_bytes != 0)
	{
		instr.args = &buffer[offset];
	}

	instr.total_bytes = *(int32_t*)&buffer[0];

	return instr;
}

WimpInstrMeta wimp_instr_get_from_node(WimpInstrNode node)
{
	return wimp_instr_get_from_buffer(node->instr.instruction, node->instr.instruction_bytes);
}