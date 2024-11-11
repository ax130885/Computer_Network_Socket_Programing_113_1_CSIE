# 定義編譯器
CXX = g++

# 定義編譯選項
CXXFLAGS = -Wall -std=c++11

# 定義目標
TARGETS = client server

# 定義目標的依賴關係和規則
all: $(TARGETS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp
	@echo "client 編譯完成"

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp
	@echo "server 編譯完成"

# 清理生成的文件
clean:
	rm -f $(TARGETS)
	@echo "清理完成"
