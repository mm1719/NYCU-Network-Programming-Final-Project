# NYCU-Network-Programming-Final-Project
## 環境建置:
按照嚴力行教授的 [unpv13e 安裝說明](https://people.cs.nycu.edu.tw/~lhyen/np/unpv13e_install.html)，將原始檔下載到同一個資料夾中，並按照教學安裝。

## server執行:
在 wsl或Ubutu的terminal 輸入
```
make server
```
server 程式即可編譯完成。
接著再輸入
```
./server
```
server就會開始執行

## client執行:
這款遊戲是雙人對決所以再另外開兩個terminal 並輸入
```
make client
```
server 程式即可編譯完成。
接著再輸入
```
./client 127.0.0.1 [username]
```
就可以加入遊戲!
