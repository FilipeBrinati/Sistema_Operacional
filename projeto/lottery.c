/*
 *  lottery.c - Implementacao do algoritmo Lottery Scheduling e sua API
 *
 *  Autores: SUPER_PROGRAMADORES_C
 *  Projeto: Trabalho Pratico I - Sistemas Operacionais
 *  Organizacao: Universidade Federal de Juiz de Fora
 *  Departamento: Dep. Ciencia da Computacao
 *
 */

#include "lottery.h"
#include <stdio.h>
#include <string.h>

// Nome unico do algoritmo. Deve ter 4 caracteres.
const char lottName[] = "LOTT";
unsigned int tot_tickets = 0;
unsigned char needs_distribute = 1;
int slot = -1;
//=====Funcoes Auxiliares=====

void addProcessToLottery(Process *p)
{
	LotterySchedParams *params;

	params = processGetSchedParams(p);
	params->begin_interval = tot_tickets;
	tot_tickets += params->num_tickets;
	params->end_interval = tot_tickets;
}

void distributeTickets(Process *plist)
{
	Process *p;
	LotterySchedParams *params;
	tot_tickets = 0;

	for (p = plist; p != NULL; p = processGetNext(p))
	{
		if (processGetStatus(p) == PROC_READY)
			addProcessToLottery(p);
	}

	needs_distribute = 0;
}

//=====Funcoes da API=====

// Funcao chamada pela inicializacao do S.O. para a incializacao do escalonador
// conforme o algoritmo Lottery Scheduling
// Deve envolver a inicializacao de possiveis parametros gerais
// Deve envolver o registro do algoritmo junto ao escalonador
void lottInitSchedInfo()
{
	SchedInfo *si = malloc(sizeof(SchedInfo));

	int i;
	for (i = 0; i < 4; i++)
	{
		si->name[i] = lottName[i];
	}

	si->initParamsFn = &lottInitSchedParams;
	si->notifyProcStatusChangeFn = &lottNotifyProcStatusChange;
	si->scheduleFn = &lottSchedule;
	si->releaseParamsFn = &lottReleaseParams;

	slot = schedRegisterScheduler(si);
}

// Inicializa os parametros de escalonamento de um processo p, chamada
// normalmente quando o processo e' associado ao slot de Lottery
void lottInitSchedParams(Process *p, void *params)
{
	schedSetScheduler(p, params, slot);
	addProcessToLottery(p);
}

// Recebe a notificação de que um processo sob gerência de Lottery mudou de estado
// Deve realizar qualquer atualização de dados da Loteria necessária quando um processo muda de estado
void lottNotifyProcStatusChange(Process *p)
{
	LotterySchedParams *params;
	int status = processGetStatus(p);

	switch (status)
	{
	case PROC_READY:
		addProcessToLottery(p);
		break;

	default:
		needs_distribute = 1;
		break;
	}
}

// Retorna o proximo processo a obter a CPU, conforme o algortimo Lottery
Process *lottSchedule(Process *plist)
{
	Process *p = plist, *chosen = NULL;
	LotterySchedParams *params;
	int chosen_ticket;

	if (needs_distribute)
		distributeTickets(plist);

	chosen_ticket = rand() % tot_tickets;

	for (p = plist; p != NULL; p = processGetNext(p))
	{
		if (processGetStatus(p) == PROC_READY)
		{
			params = processGetSchedParams(p);

			if (chosen_ticket >= params->begin_interval && chosen_ticket < params->end_interval)
			{
				chosen = p;
				break;
			}
		}
	}

	return chosen;
}

// Libera os parametros de escalonamento de um processo p, chamada
// normalmente quando o processo e' desassociado do slot de Lottery
// Retorna o numero do slot ao qual o processo estava associado
int lottReleaseParams(Process *p)
{
	int slot = processGetSchedSlot(p);
	LotterySchedParams *params = processGetSchedParams(p);
	free(params);

	return slot;
}

// Transfere certo numero de tickets do processo src para o processo dst.
// Retorna o numero de tickets efetivamente transfeirdos (pode ser menos)
int lottTransferTickets(Process *src, Process *dst, int tickets)
{
	LotterySchedParams *p1 = processGetSchedParams(src),
					   *p2 = processGetSchedParams(dst);

	int actual;
	if (p1->num_tickets < tickets)
		actual = p1->num_tickets;
	else
		actual = tickets;

	p1->num_tickets -= actual;
	p2->num_tickets += actual;

	needs_distribute = 1;
	return actual;
}
