#include <oclUtils.h>
#include <iostream>

#define NUM 150
using namespace std;

/* Estes s�o, em ordem, o nome do arquivo em que se encontra nosso kernel,
 * o n�mero de elementos da computa��o e um inteiro que ser� utilizado para
 * verifica��o de erros;
 */
const char* cSourceFile = "vector_add.cl";
cl_int iNumElements = NUM;
cl_int error = 0;

/* Esta � uma fun��o para verifica��o da computa��o na GPU
 */
void vector_add_golden(const float * const srcA, const float * const srcB, float * const golden) {
	for (int i = 0; i < iNumElements; i++)
		golden[i] = srcA[i] + srcB[i];
}

int main (int argc, char **argv) {
	/* Aqui s�o alocadas as vari�veis que residem na CPU. S�o elas os dois vetores a serem somados,
	 * e os dois vetores com os resultados da computa��o no host e no device;
	 */
	cl_float * const srcA = (cl_float*) malloc(sizeof(cl_float)*iNumElements);
	cl_float * const srcB = (cl_float*) malloc(sizeof(cl_float)*iNumElements);
	cl_float * const golden = (cl_float*) malloc(sizeof(cl_float)*iNumElements);
	cl_float * const res = (cl_float*) malloc(sizeof(cl_float)*iNumElements);
	
	/* Fazemos a devida inicializa��o dos vetores a serem somados.
	 */
	for (int i = 0; i < iNumElements; ++i) {
		srcA[i] = srcB[i] = i;
	}
	
	/* Aqui fazemos o c�lculo da soma na CPU, para fins de verifica��o.
	 */
	vector_add_golden(srcA, srcB, golden);

	/* Ok, j� temos todas as vari�veis de que precisamos neste momento, agora precisamos
	 * efetivamente come�ar a preparar a GPU para a computa��o;
	 * Inicialmente precisamos criar um CONTEXTO para a execu��o.
	 * Um contexto � o que guarda o estado de execu��o, com todos os registradores e vari�veis
	 * de uma execu��o OpenCl.
	 * Para criar o contexto, utilizaremos a seguinte fun��o:
	 cl_context clCreateContextFromType5 (cl_context_properties *properties,
											cl_device_type device_type,
											void (*pfn_notify)(const char *errinfo,
												const void *private_info, size_t cb,
												void *user_data),
											void *user_data,
											cl_int *errcode_ret)

	 * properties � um vetor de propriedades co contexto, que por hora ser� passado como NULL;
	 * cl_device_type especifica o tipo do device. A tabela completa encontra-se na especifica��o,
	 *	utilizaremos CL_DEVICE_TYPE_GPU, pois estamos criando o contexto na GPU;
	 * As proximas duas informa��es tampouco ser�o importantes por hora, e ser�o passadas como NULL;
	 * errcode_ret � o inteiro, que declaramos l� encima, que conter� o c�digo de um eventual erro;
	 *	Se tudo der certo, ser� retornado CL_SUCCESS
	 */
	cl_context context = clCreateContextFromType (NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, &error);
	if (error != CL_SUCCESS) {
		cout << "Erro criando contexto" << endl;
		exit (error);
	}
	
	/* Ok. Criamos nosso contexto!
	 * Nosso pr�ximo passo ser� criar uma COMMAND_QUEUE. Para tanto, precisaremos da lista de devices.
	 * Isso pode ser conseguido da seguinte forma:
	 * Note-se que chamamos a fun��o duas vezes: na primeira conseguimos informa��es sobre o tamanho
	 * de devemos alocar, alocamos, e logo em seguida chamamos a fun�a� novamente com esse espa�o alocado.
	 */
	cl_device_id *devices;
	size_t devices_size;
	error = clGetContextInfo (context, CL_CONTEXT_DEVICES, 0, NULL, &devices_size);
	devices = (cl_device_id*) malloc (devices_size);
	error |= clGetContextInfo (context, CL_CONTEXT_DEVICES, devices_size, devices, 0);
	if (error != CL_SUCCESS) {
		cout << "Erro pegando informacoes do contexto;" << endl;
		exit (error);
	}
	
	/* Pronto, agora j� temos um contexto e informa��es sobre ele, nos resta ainda criar uma command-queue
	 * Se os objetos e a mem�ria � gerenciada por um contexto em OpenCl, opera��es sobre estes objetos e mem�ria
	 *	s�o feitos usando a command_queue. � uma fila de comandos a serem executados; M�ltiplas queues permitem
	 *	m�ltiplos comandos independentes sem requerimento de sincroniza��o, a menos de compartilhamento de objetos;
	 */
	cl_command_queue queue = clCreateCommandQueue (context, *devices, 0, &error);
	if (error != CL_SUCCESS) {
		cout << "Erro criando command_queue" << endl;
		exit (error);
	}
	
	/* Ok, j� criamos as estruturas necess�rias para gerenciar a execu��o OpenCL
	 * Vamos agora alocar os vetores para serem somados na GPU, e em seguida invocar o kernel;
	 *
	 * Em OpenCl, temos que alocar um buffer, que � um espa�o de mem�ria a ser alocado em um contexto.
	 * A aloca��o tem a seguinte assinatura:

	 cl_mem clCreateBuffer (cl_context context,
							cl_mem_flags flags,
							size_t size,
							void *host_ptr,
							cl_int *errcode_ret)

	 * As flags podem ser:
		CL_MEM_READ_WRITE
		CL_MEM_WRITE_ONLY
		CL_MEM_READ_ONLY
		CL_MEM_USE_HOST_PTR
		CL_MEM_ALLOC_HOST_PTR
		CL_MEM_COPY_HOST_PTR

		E devem ser combinadas usando |
	 */
	cl_int error2 = 0, error3 = 0;
	cl_mem srcA_d = clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * iNumElements,
		(void*)srcA, &error);
	cl_mem srcB_d = clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * iNumElements,
		(void*)srcB, &error2);
	cl_mem res_d = clCreateBuffer (context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * iNumElements, NULL, &error3);
	if ((error | error2 | error3) != CL_SUCCESS) {
		cout << "Erro alocando memoria no host" << endl;
		exit (error|error2|error3);
	}

	/* Ok, criamos os Buffers, agora precisamos criar um programa e compil�-lo.
	 * Faremos isso usando:
	 cl_program clCreateProgramWithSource (cl_context context,
											cl_uint count,
											const char **strings,
											const size_t *lengths,	
											cl_int *errcode_ret)

	 * para criar o programa, e:

	 cl_int clBuildProgram (cl_program program,
							cl_uint num_devices,
							const cl_device_id *device_list,
							const char *options,
							void (*pfn_notify)(cl_program, void *user_data),
							void *user_data)

	 * para compila-lo;
	 */
	// Estas tr�s linhas apenas conseguem o endere�o do kernel
	size_t kernelLength;
    const char* cPathAndName = shrFindFilePath(cSourceFile, argv[0]);
    const char* cSourceCL = oclLoadProgSource(cPathAndName, "", &kernelLength);
	cl_program program = clCreateProgramWithSource (context, 1, (const char**)&cSourceCL, &kernelLength, &error);
	if (error != CL_SUCCESS) {
		cout << "Erro criando o programa" << endl;
		exit (error);
	}
	error = clBuildProgram (program, 0, NULL, NULL, NULL, NULL);
	if (error != CL_SUCCESS) {
		cout << "Erro buildando o programa" << endl;
		exit (error);
	}

	/* Precisamos agora criar um 'kernel object', que deve ser associado ao programa que acabamos
	 * de criar;
	 * Isso pode ser feito utilizando:
	 
	 cl_kernel clCreateKernel (cl_program program,
								const char *kernel_name,
								cl_int *errcode_ret)
	 */
	cl_kernel vector_add_kernel = clCreateKernel (program, "vector_add", &error);
	if (error != CL_SUCCESS) {
		cout << "Erro criando o kernel" << endl;
		exit (error);
	}

	/* Ok, j� criamos um contexto, descobrimos os devices, criamos um command_queue, 
	 * alocamos a mem�ria, criamos um programa e associamos os kernels.
	 * Basta agora chamar os kernels para que esteja tudo conclu�do.
	 * 
	 * Para fazer isso, devemos primeiramente empilhar os argumentos. Isso pode ser feito
	 * usando:
	 cl_int clSetKernelArg (cl_kernel kernel,
							cl_uint arg_index,
							size_t arg_size,
							const void *arg_value)
	 */
	error = clSetKernelArg (vector_add_kernel, 0, sizeof(cl_mem), (const void*)&srcA_d);
	error |= clSetKernelArg (vector_add_kernel, 1, sizeof(cl_mem), (const void*)&srcB_d);
	error |= clSetKernelArg (vector_add_kernel, 2, sizeof(cl_mem), (const void*)&res_d);
	error |= clSetKernelArg (vector_add_kernel, 3, sizeof(cl_int), (const void*)&iNumElements);
	if (error != CL_SUCCESS) {
		cout << "Error empilhando os parametros" << endl;
		exit (error);
	}

	/* Aqui declaramos o numero de work-items por work-group e o n�mero de work-items total.
	 * shrRoundUp arredonda o numero de elementos para o maior multiplo do numero de work-items por work-group;
	 */
	size_t work_groups = 256;
    size_t work_items = shrRoundUp((int)work_groups, iNumElements);

	/* Ok, empilhamos os parametros, agora vamos finalmente chamar esse kernel.
	 * Isso pode ser feito usando-se:
	 cl_int clEnqueueNDRangeKernel (cl_command_queue command_queue,
									cl_kernel kernel,
									cl_uint work_dim,
									const size_t *global_work_offset,
									const size_t *global_work_size,
									const size_t *local_work_size,
									cl_uint num_events_in_wait_list,
									const cl_event *event_wait_list,
									cl_event *event)
	 * Aqui chamamos a aten��o para global_work_size, que � o n�mero de work-items e para local-work-size, que � o n�mero de
	 * work-item por work-group;
	 */
	error = clEnqueueNDRangeKernel (queue, vector_add_kernel, 1, NULL, &work_items, &work_groups, 0, NULL, NULL);
	if (error != CL_SUCCESS) {
		cout << "Erro empilhando os kernels" << endl;
		exit (error);
	}
	/* Excelente! J� lan�amos nosso kernel (220 linhas depois :P)
	 * Agora temos que conseguir de volta o resultado. Para isso, devemos enfileirar uma requisi��o
	 * de leitura de buffer. Isso pode ser feito da seguinte maneira:
	
	 cl_int clEnqueueReadBuffer (cl_command_queue command_queue,
								cl_mem buffer,
								cl_bool blocking_read,
								size_t offset,
								size_t cb,
								void *ptr,
								cl_uint num_events_in_wait_list,
								const cl_event *event_wait_list,
								cl_event *event)
	 */
	error = clEnqueueReadBuffer (queue, res_d, CL_TRUE, 0, sizeof(cl_float)*iNumElements, (void *)res, 0, NULL, NULL);
	if (error != CL_SUCCESS) {
		cout << "Erro lendo o buffer" << endl;
		exit (error);
	}
	
	/* Agora simplesmente fazemos o check com os valores computados na CPU, se tudo estiver ok, maravilha!
	 */
	for (int i = 0; i < iNumElements; ++i) {
		if (golden[i] != res[i]) {
			cout << "Nao fecha a computacao" << endl;
			exit (1);
		}
	}
	cout << "Passou no TESTE!!!" << endl;
	return 0;

}