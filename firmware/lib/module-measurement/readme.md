# Meas Task and Class architecture

Class:
	- has no knowledge about RTOS,
	- has no knowledge about device data comes from,
	- performs measurement filtering and averaging,
	
Task:
	- connects driver (ADC) with Meas Class,
	- shares task API, so modules like CLI can manipulate with Meas Class parameters,

updates:
	- i need to update this tasks API and task itself,