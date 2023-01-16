






// int main(){
//     int dd = 5 + 1 * 2;
// }; // 必须加; 要不然出错哦


// int cc;
// cc = 1 +2;

// int  b = 5;
// int d = 4;


// 对于函数来说 变量只有自己的和全局的
// 自己的变量要么是rax 要么是rbp的偏移 而全局变量是不能用rbp的 rbp只存在于frame中 全局变量直接用地址
// 类似函数

int out = 3;

int function(int a, int c){
    // int d = 4 + a + c;
    // int kk = 5 * c;
    int tt = a * 3 + c*2/3 +4*a; // 这里的out 属于全局
    return tt;
    // int s = c -2;
    // return d + s;
    // int s = 3 + c;
    // return d+s;
};


// int mainC(int d, int a, int aa, int aaa, int aaaa){
//     int s =d*3 + function(3, 4);
//     return s;
// }

int mainC(int d, int a, int aa, int aaa, int aaaa, int aaaaa, int ab, int vvv){
    int s =d*3 + function(3, 4);
    return s;
}
// // 对于fileNode compound body的顺序 后续在根据line调整
// int cc = 1;
// cc = 1  + 5- 9* 10 * 9 - 2;



int KK(int a, int d){
    int s = mainC( 1,  2,  3,  4,  5,  6,  7,  d);
    return s;
}
