//							ЗАДАНИЕ
/*	На вход консольному приложению дается путь к каталогу с файлами и имя файла с результатами.
	В каждом файле строка и набор разделителей. Нужно разбить строку на подстроки по этим
	разделителям и вывести результат в выходной файл. Парсить файлы надо в несколько потоков и
	результат должен быть помещен в один выходной файл. */

//							ИНСТРУКЦИЯ
/*
* Программа запрашивает путь к файлам, которые необходимо распарсить.
* Примером служат файлы ab.txt и hd.txt, находящиеся в корневой папке.
* Файлы запрашиваются до тех пор, пока не придет пустая строка.
* В запросе разделители необходимо так же вводить через Enter.
* Примером могут служить "!?."
* 
* Количество потоков зависит от количества обрабатываемых файлов.
* Каждый поток работает с отдельным файлом.
* Для удобства в выходной файл и консоль записываются 
* подстрока, файл, откуда она взята и номер подстроки.
* 
* Результат записывается в выходной файл out.txt, находящийся в корневой папке.
*/

#include <iostream>
#include <fstream>

#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

//Настройка синхронизации
std::mutex mtx;

//Функция разделения на подстроки
void Split(std::string path, std::string nameOut, std::vector<std::string> bufferDelimetr)
{
	std::ifstream inFile(path);											//входной файл
	std::fstream outFile(nameOut, std::ios_base::in | std::ios_base::app);										//выходной файл


	std::string delimetr;												//разделитель
	for (const auto& it : bufferDelimetr) delimetr += it;				//преобразование разделителей из vector <string> в string


	if (!inFile.is_open())
	{
		mtx.lock();
		std::cout << "ERROR:: File " << path << " does not exist" << std::endl;
		mtx.unlock();
	}
	else
	{
		//Чтение данных из файла
		std::string str;												//строка файла
		int i = 0;
		getline(inFile, str, '\0');

		std::string token;												//подстрока файла

		size_t beg, pos = 0;
		while ((beg = str.find_first_not_of(delimetr, pos)) != std::string::npos)
		{
			i += 1;														//номер подстроки/итерации потока

			pos = str.find_first_of(delimetr, beg + 1);
			token = str.substr(beg, pos - beg);
			mtx.lock();

			//Вывод на консоль
			std::cout << "\tFile:\t" << path << "\tIteration:\t" << i << std::endl;
			std::cout << token << std::endl;

			//Вывод в выходной файл
			outFile << "\tFile:\t" << path << "\tSubstring number:\t" << i << std::endl;
			outFile << token << std::endl;

			mtx.unlock();
			
			std::this_thread::sleep_for(std::chrono::milliseconds(500)); //демонстрация, что потоки работают одновременно
		}
	}

	std::cout << std::endl;
	std::cout << "\tFile:\t" << path << "\tis DONE\t" << std::endl;
	std::cout << std::endl;

	inFile.close();
	outFile.close();
}

int main()
{
	setlocale(LC_ALL, "ru");								//на всякий случай :)

	std::string path = "";									//путь с консоли
	std::string del = "";									//разделитель с консоли

	std::string nameOut = "out.txt";						//название выходного файла
	std::ofstream outFile(nameOut);							//выходной файл

	std::vector<std::string> bufferPath;					//векторный набор путей с консоли
	std::vector<std::string> bufferDelimetr;				//векторный набор разделителей с консоли
	std::vector<std::thread> bufferThread;					//векторный набор потоков с консоли

	//Запрос обрабатываемых файлов с консоли
	do
	{
		std::cout << "Enter the path to the directory with file to split: ";
		std::getline(std::cin, path);
		if (!path.empty())
		{
			bufferPath.push_back(path);
		}
	} while (!path.empty());

	unsigned int vector_size = bufferPath.size();			//количество введенных путей
	bufferThread.reserve(vector_size);						//резервировуем место для потоков равное количеству путей
	
	//Запрос разделителей с консоли
	do
	{
		std::cout << "Enter delimiter: ";
		std::getline(std::cin, del);
		if (!del.empty())
		{
			bufferDelimetr.push_back(del);
		}
	} while (!del.empty());

	//Передача массиву потоков функции со значениями					
	for (uint32_t i = 0; i < vector_size; ++i)
	{
		bufferThread.emplace_back(std::thread(Split, bufferPath[i], nameOut, bufferDelimetr)); //аргументы - конкретный поток, имя выходного файла, набор разделителей
	}
	
	//Присоединение потоков к главному потоку
	for (auto& thread : bufferThread) { if (thread.joinable()) { thread.join(); } }

	return 0;
}