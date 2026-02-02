# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(IntegrationSystem)

# Стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows нужны библиотеки Winsock
if(WIN32)
    set(EXTRA_LIBS ws2_32)
else()
    set(EXTRA_LIBS pthread)
endif()

#СЕРВЕР
add_executable(IntegrationServer 
    IntegrationServer.cpp  
)

target_link_libraries(IntegrationServer ${EXTRA_LIBS})

#КЛИЕНТ 1 (прямоугольники)
add_executable(IntegrationClient1
    IntegrationClient1.cpp   
    calculator.cpp         
)

target_link_libraries(IntegrationClient1 ${EXTRA_LIBS})

#КЛИЕНТ 2 (трапеции)
add_executable(IntegrationClient2
    IntegrationClient2.cpp   
    calculator2.cpp          
)

target_link_libraries(IntegrationClient2 ${EXTRA_LIBS})

# Для многопоточности
find_package(Threads REQUIRED)
target_link_libraries(IntegrationClient1 Threads::Threads)
target_link_libraries(IntegrationClient2 Threads::Threads)

# Сообщение в консоль
message("=== Проект собран успешно ===")
message("  Сервер: IntegrationServer.exe")
message("  Клиент 1 (прямоугольники): IntegrationClient1.exe")
message("  Клиент 2 (трапеции): IntegrationClient2.exe")