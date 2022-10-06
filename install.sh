mkdir Client/StockMarketClient/build
cd Client/StockMarketClient/build
cmake ..
make install
cd -
mkdir Server/StockMarketServer/build
cd Server/StockMarketServer/build
cmake ..
make install
cd -
mkdir Common/tests/StockMarketTest/build
cd Common/tests/StockMarketTest/build
cmake ..
make install
cd -



