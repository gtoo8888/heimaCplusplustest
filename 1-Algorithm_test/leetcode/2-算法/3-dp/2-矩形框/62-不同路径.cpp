#include <iostream>
#include <vector>
using namespace std;
#include "C:\\Users\\Yan\\Desktop\\languguetest\\Cplusplustest\algorithm_util.h"

class Solution {
public:
    int uniquePaths(int m, int n) {
        vector<vector<int> > dp(m,vector<int>(n,0));
		for(int i = 0;i < m;i++)
			dp[i][0] = 1;
		for(int i = 0;i < n;i++)
			dp[0][i] = 1;
		for(int i = 1;i < m;i++){
			for(int j = 1;j < n;j++) {
				dp[i][j] = dp[i - 1][j] + dp[i][j - 1];
			}
		}
        PrintVVector(dp);
		return dp[m - 1][n - 1];
    }
};
		
int main() {
	Solution solution;
	
    int m = 3;
    int n = 7;

	int ans = 0;
	ans = solution.uniquePaths(m,  n);
	
	cout << ans << endl;
	return 0;
}



