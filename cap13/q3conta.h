/* Deposita um valor na conta, soma no saldo, nunca bloqueia. */

extern void deposita( double valor);

/* Retira o valor da conta, subtrai do saldo, fica bloqueado até que
exista saldo suficiente na conta para atender a retirada. Pedidos de
retirada são atendidos por ordem de chegada. */

extern void retira( double valor);

/* Bloqueia a thread até que o somatório dos depósitos, sem
considerar as retiradas, ultrapasse o valor_vip. */

extern void espera_saldo_vip( double valor_vip);

/* Aplica sobre o saldo da conta uma correção monetária, fornecida
em pontos percentuais.*/

extern void aplica_correcao( double correcaoPP);
