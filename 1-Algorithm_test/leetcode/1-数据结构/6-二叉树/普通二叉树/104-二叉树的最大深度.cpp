#include <iostream> 
using namespace std;
#include <vector> 
#include <string>
#include <queue>
#include "E:\\Desktop\\languguetest\\Cplusplustest\\algorithm_util.h"


class Solution {
public:
    int maxDepth(TreeNode* root) {
		queue<TreeNode*> que;
		int depth = 0;
		if(root != nullptr)	que.push(root);
		while(!que.empty()) {
			int size = que.size();
			depth++;
			for(int i = 0;i < size;i++) {
				TreeNode* temp = que.front();que.pop();
				if(temp->left) que.push(temp->left);
				if(temp->right) que.push(temp->right);
			}
		}
		return depth;
    }
};


int main() {
	Solution solution;
	int ans;
	vector<int> data = {3,9,20,null,null,15,7};
	TreeNode* root = CreateBinaryTree(data);
	
//	PrePrint(root);
	ans = solution.maxDepth(root);
	
	cout << ans << endl;
	
	return 0;
}



