#include <serial.h>
namespace embeded_data {

using namespace tinychain;
serial::serial(const char* uart, int baud_rate):uart(uart), baud_rate(baud_rate){
	//if ((fd = open(uart, O_RDWR | O_CREAT, 0777)) < 0){	//�򿪴���
	if ((fd = open(uart, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {	//�򿪴���
		log::error(__FUNCTION__) << "open error\n";
		init_ok = false;
		return;
	}
	else {
		init_serial();
		init_ok = true;
	}

}

serial::~serial() {

	if (thread_t > 0) {
		pthread_cancel(thread_t);
		pthread_join(thread_t, NULL);
	}
	close(fd);
}

int serial::init_serial(void) {
	 
	if (tcgetattr(fd, &options) < 0)	//��ȡ�����ն�����
	{
		log::error("serial") << "tcgetattr error\n";
		return -1;
	}
	cfmakeraw(&options);   //config to raw mode   shabi code
	/*���ÿ���ģʽ*/
	//options.c_cflag |= CLOCAL;//��֤����ռ�ô���
	//options.c_cflag |= CREAD;//��֤������ԴӴ����ж�ȡ����

	/*��������λ:��λ*/
	//options.c_cflag &= ~CSIZE;//����������־λ
	//options.c_cflag |= CS8;  //��λ

	/*����У��λ����У��*/
	//options.c_cflag &= ~PARENB;//PARENB��������żλ��ִ����żУ��
	//options.c_cflag &= ~INPCK;//INPCK��ʹ��żУ��������

	/*����ֹͣλ��һλ*/
	//options.c_cflag &= ~CSTOPB;//CSTOPB��ʹ��һλֹͣλ

	/*���õȴ�ʱ�����С�����ַ�*/
	//options.c_cc[VTIME] = 0;//������select������
	//options.c_cc[VMIN] = 1;//���ٶ�ȡһ���ַ�
	 
	/*��մ���Buff*/
	//tcflush(fd, TCIFLUSH);

	/*���ò�����*/
	switch (baud_rate) {
	case 4800:
		cfsetispeed(&options, B4800);
		cfsetospeed(&options, B4800);
		break;
	case 9600:
		cfsetispeed(&options, B9600);
		cfsetospeed(&options, B9600);
		break;
	case 19200:
		cfsetispeed(&options, B19200);
		cfsetospeed(&options, B19200);
		break;
	case 38400:
		cfsetispeed(&options, B38400);
		cfsetospeed(&options, B38400);
		break;
	case 115200:
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
		break;
	default:
		log::error("serial") << "Unkown baude!\n";
		return -1;
	}

	/*��������*/
	if (tcsetattr(fd, TCSANOW, &options) < 0)
	{
		log::error("serial") << "tcsetattr failed\n";
		return -1;
	}
	log::info("serial") << "initialize serial port successful\n";
	return 0;
}

void serial::info(void) {
	std::cout << "serial " << uart << ": baud rate = " << baud_rate
				<<". init:"<<init_ok << std::endl;
}

int serial::write_data(const std::string string) {
	const char *str = string.c_str();
	int length = string.size();
	int nwritten;

	if (!init_ok) {
		log::error(__FUNCTION__) << "init error!\n";
		return -1;
	}
	nwritten = write_data(str, length);
	log::info(__FUNCTION__) << "wirtten: " << nwritten;
	return nwritten;
}

/**
@brief ʹ�ô��ڷ����ַ���
@param src �ַ����׵�ַ
@parama length ��Ҫ���͵ĳ���
*/
int serial::write_data(const char* str, int length) {
	int nwritten = 0;
	if (!init_ok) {
		log::error(__FUNCTION__) << "init error!\n";
		return -1;
	}
	while (nwritten != length) { //δ����ʱ����
		write(fd, str+ nwritten, sizeof(char));
		++nwritten;
	}
	return nwritten;
}

void serial::start_read(void) {
	if (!init_ok) {
		log::error(__FUNCTION__) << "init error!\n";
		return;
	}
	if(pthread_create(&thread_t, NULL, &read_thread, &fd) != 0) {
		log::error(__FUNCTION__) << "pthread create error!\n";
	}
	else {
		//pthread_detach(thread_t); //�߳̽����Զ��ͷ���Դ
	}
}

void* serial::read_thread(void *_fd) {
	unsigned char buff[1024];
	int fd = *(int*)_fd;
	int nRead = 0;
	fd_set rd;

	//pthread_detach(pthread_self()); //�߳̽����Զ��ͷ���Դ
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //���ñ��̶߳�Cancel�źŵķ�Ӧ  CANCEL_ENABLE
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //���ñ��߳�ȡ��������ִ��ʱ�� ����ȡ��
	while (1) {
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		select(fd + 1, &rd, NULL, NULL, NULL);
		read(fd, buff + nRead, sizeof(char));
		if (buff[nRead] == '\n') {
			//������һ֡���ݹ���Ĳ���
			//�����Ȳ������������
			short int x,  y;
			int i;
			printf("Recv:");
			for (i = 0; i < nRead; i++) {
				printf("%02x ", buff[i]);
			}
			printf("\n");
			//memcpy(&x, buff + 5, 2);
			//memcpy(&y, buff + 7, 2);
			//x = (short int)(buff[5]) << 8 + (short int)buff[6];
			//y = (short int)(buff[7]) << 8 + (short int)buff[8];
			x = (buff[5] << 8) | buff[6];
			y = (buff[7] << 8) | buff[8];
			printf("x=%hd, y=%hd, sizeof(*x)=%d \n", x, y, sizeof(short int));
			printf("x=%4x, y=%4x \n\n", x, y);
			if (nRead < 1023)	buff[nRead + 1] = 0;
			//log::info(__FUNCTION__) << "recv msg: " << buff;
			nRead = 0;
		}
		else {
			++nRead;
		}

		if (nRead >= 1023) {
			nRead = 1023;
		}

	}
}
}

