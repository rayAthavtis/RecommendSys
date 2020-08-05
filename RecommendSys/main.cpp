//
//  main.cpp
//  RecommendSys
//
//  Created by 任蕾 on 2020/6/11.
//  Copyright © 2020 ray. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <math.h>

#include <sys/time.h>

using namespace std;

// users 19835
#define USERS 20000
#define NUM 5000
#define K 3

double predict(vector<map<int, double>> &train, int user, int item) {
    vector<double> sim;
    vector<double> item_score;
    double for_avg = 0.0;
    vector<double> avg;
    for (int i=0; i<train.size(); i++) {
        // 找对item有评分的用户
        if (train[i].count(item)==0) { continue; }
        vector<double> common1, common2;
        int n = 0;
        double sum1 = 0.0;
        double sum2 = 0.0;
        map<int, double>::iterator it;
        int cnt = 0;
        for (it=train[user].begin(); cnt<train[user].size(); ++it, cnt++) { // 找评过的分
            int id = it->first;
            if (train[i].count(id)==0) { continue; }
            double tmp1 = it->second;  // user对id的评分
            double tmp2 = train[i][id];  // i对id的评分
            // if (tmp1==0 || tmp2==0) { continue; } // 评分为0
            common1.push_back(tmp1);
            common2.push_back(tmp2);
            sum1 += tmp1;
            sum2 += tmp2;
            n++;
        }
        if (n==0)
        {
            // 没有公共的
            continue;
        };
        double mean1 = sum1 / n;
        double mean2 = sum2 / n;
        for_avg = mean1;
        double norm1 = 0.0, norm2 = 0.0, s = 0.0;
        //bool avg0 = true;
        for (int j=0; j<n; j++) {
            double tmp1 = common1[j] - mean1;
            //if (tmp1!=0) { avg0 = false; }
            double tmp2 = common2[j] - mean2;
            norm1 += pow(tmp1, 2);
            norm2 += pow(tmp2, 2);
            s += tmp1 * tmp2;
        }
        // user公共部分打分都一样
        //if (avg0) { continue; }
        
        double norm = sqrt(norm1) * sqrt(norm2);
        if (s==0) { continue; }  // 相似度为0的
         avg.push_back(mean2);
        double sc = train[i][item];
        item_score.push_back(sc);  // i对item的评分
        
        if (norm==0)
        {
            sim.push_back(1);
        }
        else
            // 完成一个相似用户的处理
            sim.push_back(s/norm);
    }
    double su = 0.0, ss = 0.0;
    //if (sim.size()==0) { return for_avg;}
    /*
    int k=0;
    for (int i=0; i<sim.size(); i++) {
    // 阈值设置为0
        if(abs(sim[i])>0)
        {
            su += sim[i];
            ss += sim[i] * item_score[i];
            k++;
        }
    }
    if(k>0)
    {
        sim.clear();
        item_score.clear();
        return ss / su;
    }
    */
    double su2 = 0.0, ss2 = 0.0;
    //if (sim.size()==0) { return for_avg;}
    int k=0, k2=0;
    for (int i=0; i<sim.size(); i++) {
        // 阈值设置为0
        if(sim[i]>0.4)
        {
            su += sim[i];
            ss += sim[i] * (item_score[i] - avg[i]);
            k++;
        }
        if(sim[i]<-0.6)
        {
            su2 += sim[i];
            ss2 += sim[i] * (item_score[i] - avg[i]);
            k2++;
        }
    }
    if(k>0 && k2==0) {
        return for_avg + ss / su;
    }
    else if (k==0 && k2>0) {
        return for_avg - ss2 / su2;
    }
    else if (k>0 && k2>0) {
        return ((for_avg + ss / su)*k + (for_avg - ss2 / su)*k2 ) / (k+k2);
    }
    else
    {
        int cnt=0;
        double sum=0;
        map<int, double>::iterator it;
        for (it=train[user].begin(); cnt<train[user].size(); ++it, cnt++)
        { sum+=it->second; }
        return sum/train[user].size();
    }
    
}

bool UserBasedCF(string inf, string testf) {
    ifstream file(inf);  // 内容读取到file中
    // 确定文件打开
    if (!file) { cout<<"open \""<<inf<<"\" failed! "<<endl; return false; }
    
    vector<map<int, double>> train_data;
    
    int user_size = 0;  // 用户数目
    char aline[50];
    set<int> items;
    //评分数
    int trainnum=0;
    
    while (file>>aline) {
        strtok(aline, "|");
        string tmp2 = strtok(NULL, "|");
        int item_num = stoi(tmp2);
        map<int, double> tmp;
        for (int i=0; i<item_num; i++) {  // 读入记录
            int item;
            double score;
            file>>item>>score;
            items.insert(item);
            //if (score==0) { continue; }
            tmp[item] = score;
        }
        train_data.push_back(tmp);
        user_size++;
        trainnum+=item_num;
        memset(aline, 0, 50);
    }
    cout<<"训练集用户数目："<<user_size<<endl;
    cout<<"训练集项目数目："<<items.size()<<endl;
    cout<<"训练集评分数目："<<trainnum<<endl;
    file.close();
    
    fstream file2(testf);
    // 确定文件打开
    if (!file2) { cout<<"open \""<<testf<<"\" failed! "<<endl; return false; }
    
    ofstream result;
    string ofname = "result.txt";
    result.open(ofname);
    if (!result) { cout<<"open \"result.txt\" failed! "<<endl; return false; }
    
    // vector<map<int, double>> predict_data;
    int no = 0;
    // 评分数
    int num = 0;
    // 项目数
    set<int> testitems;
    
    //计时
    struct timeval t1,t2;
    double fuse;
    gettimeofday(&t1,NULL);
    
    while (file2>>aline) {
        strtok(aline, "|");
        string tmp2 = strtok(NULL, "|");
        int item_num = stoi(tmp2);
        
        result<<no<<"|"<<item_num<<endl;
        map<int, double> tmp;
        for (int i=0; i<item_num; i++) {
            int item;
            file2>>item;
            testitems.insert(item);
            // test里user也是按顺序的
            if (train_data[no].count(item)==0)  // 没有
            { tmp[item] = predict(train_data, no, item); }
            else { tmp[item] = train_data[no][item]; }
            // cout<<item<<" "<<tmp[item]<<endl;
            result<<item<<" "<<tmp[item]<<endl;
            num++;
        }
        // predict_data.push_back(tmp);
        no++;
        memset(aline, 0, 50);
        cout<<"complete: "<<no<<endl;
    }
    
    gettimeofday(&t2,NULL);
    fuse = (t2.tv_sec-t1.tv_sec)+(double)(t2.tv_usec-t1.tv_usec)/1000000.0;
    cout<<"测试集用户数目："<<no<<endl;
    cout<<"测试集项目数目："<<testitems.size()<<endl;
    cout<<"测试集评分数目："<<num<<endl;
    cout<<"time: "<<fuse<<endl;
    cout<<"average time: "<<fuse/num<<endl;
    
    file2.close();
    result.close();
    
    return true;
}

double RMSE(string RMSEtest, string result) {
    ifstream file(RMSEtest);  // 实际值，内容读取到file中
    // 确定文件打开
    if (!file) { cout<<"open \""<<RMSEtest<<"\" failed! "<<endl; return false; }
    
    vector<double> t_data;  // 顺序一样，不用记录item
    char aline[50];
    
    while (file>>aline) {
        strtok(aline, "|");
        string tmp2 = strtok(NULL, "|");
        int item_num = stoi(tmp2);
        //vector<double> tmp(item_num);
        for (int i=0; i<item_num; i++) {  // 读入记录
            int item;
            double score;
            file>>item>>score;
            // if (score==0) { continue; }
            //tmp[i] = score;
            t_data.push_back(score);
        }
        
        memset(aline, 0, 50);
    }
    file.close();
    
    //cout<<t_data[0]<<" "<<t_data[0]<<endl;
    
    ifstream file2(result);  // 预测值，内容读取到file中
    // 确定文件打开
    if (!file2) { cout<<"open \""<<result<<"\" failed! "<<endl; return false; }
    
    vector<double> r_data;  // 顺序一样，不用记录item
    
    while (file2>>aline) {
        strtok(aline, "|");
        string tmp2 = strtok(NULL, "|");
        int item_num = stoi(tmp2);
        vector<double> tmp(item_num);
        for (int i=0; i<item_num; i++) {  // 读入记录
            int item;
            double score;
            file2>>item>>score;
            //tmp[i] = score;
            r_data.push_back(score);
        }
        
        memset(aline, 0, 50);
    }
    file2.close();
    
    //cout<<r_data[0]<<" "<<r_data[1]<<endl;
    
    
    long double sum = 0.0;
    int num = 0;
    
    for (int i=0; i<t_data.size(); i++) {
        //cout<<t_data[i]<<endl;
        //cout<<r_data[i]<<endl;
            sum += pow(t_data[i]-r_data[i], 2);
        //cout<<"sum:"<<sum<<endl;
            num++;
    }
    
    double ret = sqrt(sum / num);
    return ret;
}

int main(int argc, const char * argv[]) {
    // 使用相对路径，在Xcode->product->scheme->edit->run->options->working directory修改工作目录
    string tfile = "traint.txt";
    string fileName = "testt.txt";
    
    // UserBasedCF(tfile, fileName);
    
    if (fileName=="testt.txt") {
        // string rfile = "RMSEreal.txt";
        string rfile = "rmse.txt";
        double rmse = RMSE(rfile, "result.txt");
        cout<<"RMSE: "<<rmse<<endl;
    }
    
    return 0;
}
