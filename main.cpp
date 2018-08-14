//
//  main.cpp
//  Daksh_challenge
//
//  Created by Fan Zhongyan on 14/8/2018.
//  Copyright © 2018 Fan Zhongyan. All rights reserved.
//
#include<iostream>
#include<vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>


using namespace std;
class Execution
{
public:
    int time;
    double tradePrice;
    int tradeSize;
    double theoreticalPrice;
    double deltaTraded;
    double basePx;
    int Pos;
    double vola;
    double comms;
    
    Execution(int t, double tp0, int ts, double tp1, double dt, double bp, double p, double v, double c)
    {
        this->time = t;
        this->tradePrice = tp0;
        this->tradeSize = ts;
        this->theoreticalPrice = tp1;
        this->deltaTraded = dt;
        this->basePx = bp;
        this->Pos = p;
        this->vola = v;
        this->comms = c;
    }
};

class Trade{
public:
    int time;
    double tradePrice;
    int totalTradeSize;
    string tradeType;
    
    Trade(int t, double tp, int tts, string tt)
    {
        this->time = t;
        this->tradePrice = tp;
        this->totalTradeSize = tts;
        this->tradeType = tt;
    }
};

void str2int(int &int_temp,const string &string_temp)
{
    stringstream stream(string_temp);
    stream>>int_temp;
}

void str2double(double &double_temp,const string &string_temp)
{
    stringstream stream(string_temp);
    stream>>double_temp;
}

void splitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));
        
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

static void getGreaterThanEqMatch(const std::vector<double>& aSearchFor, const std::vector<double>& aSearchIn, std::vector<int>& aRes)
{
    for(int i = 0; i < aSearchFor.size(); i++)
    {
        int res = -1;
        double min = aSearchFor[i];
        for(int j = 0; j < aSearchIn.size(); j++)
        {
            if(aSearchIn[j] >= aSearchFor[i])
            {
                if(res == -1)
                {
                    min = aSearchIn[j];
                    res = j;
                }else if(min > aSearchIn[j])
                {
                    min = aSearchIn[j];
                    res = j;
                }
            }
        }
        aRes.push_back(res);
    }
}

static int initExecData(string path, vector<Execution>& execs)
{
	fstream fin(path.c_str());
    string myLine;
    if (fin.is_open() == false)
    {
        return 1;
    }
    
    while(getline(fin,myLine))
    {
        vector<string> myString;
        splitString(myLine, myString, ",");
        if(myString[0].compare("Time") != 0 && myString.size() == 9)
        {
            int t, ts,  p;
            double tp0, tp1, dt, bp, v, c;
            str2int(t, myString[0]);
            str2double(tp0, myString[1]);
            str2int(ts, myString[2]);
            str2double(tp1, myString[3]);
            str2double(dt, myString[4]);
            str2double(bp, myString[5]);
            str2int(p, myString[6]);
            str2double(v, myString[7]);
            str2double(c, myString[8]);
            Execution exec(t, tp0, ts, tp1, dt, bp, p, v, c);
            execs.push_back(exec);
        }
    }
    fin.close();
    return 0;
}

static int initTradeData(string path, vector<Trade>& trades)
{
	fstream fin(path.c_str());
    string myLine;
    if (fin.is_open() == false)
    {
        return 1;
    }
    
    while(getline(fin,myLine))
    {
        vector<string> myString;
        splitString(myLine, myString, ",");
        if(myString[0].compare("Time") != 0 && myString.size() == 4)
        {
            int t, tts;
            double tp;
            string tt = myString[3];
            str2int(t, myString[0]);
            str2double(tp, myString[1]);
            str2int(tts, myString[2]);
            Trade my_trade(t, tp, tts, tt);
            trades.push_back(my_trade);
        }
    }
    fin.close();
    return 0;
}



void getResults(const vector<Execution>& execs, const vector<Trade>& trades,  const vector<int>& ref, double& tswc, double& tdt, double& tc, double& dca)
{
    int tradesize_sum = 0;
    for(int i = 0; i < execs.size(); i++)
    {
        tradesize_sum = tradesize_sum + execs[i].tradeSize;
    }
    
    
    for(int i = 0; i < execs.size(); i++)
    {
        int id = ref[i];
        double mult = 1;
        if(trades[id].tradeType.compare("Quote") == 0)mult = 1.75;
        else if (trades[id].tradeType.compare("Hit") == 0)mult = 1.5;
        else if (trades[id].tradeType.compare("Depth") == 0)mult = 2.5;
        else mult = 1;
        
        dca = dca + ((double)trades[id].totalTradeSize * exp(execs[i].tradePrice - execs[i].theoreticalPrice))/mult;
        tswc = (double)abs(execs[i].tradeSize) * exp(execs[i].tradePrice - execs[i].theoreticalPrice)/(double)abs(tradesize_sum);
        tdt = tdt + abs(execs[i].deltaTraded);
        tc = tc + execs[i].comms;
    }
}




int main(int argc, const char * argv[]) {
    
    
    string executionPath;
    string tradePath;
    
    cout<<"Input path of execution data."<<endl;
    cin>>executionPath;
    //executionPath = "/Users/zyfan/Desktop/1.csv";
    cout<<"Input path of trade data."<<endl;
    cin>>tradePath;
    //tradePath = "/Users/zyfan/Desktop/2.csv";
    
    vector<Execution> execs;
    vector<Trade> trades;
    initExecData(executionPath, execs);
    initTradeData(tradePath, trades);
    
    vector<double> sfor;
    vector<double> sin;
    vector<int> ref;
    
    for(int i = 0; i < execs.size(); i++)
    {
        double tmp = execs[i].time;
        sfor.push_back(tmp);
    }
    for(int i = 0; i < trades.size(); i++)
    {
        double tmp = trades[i].time;
        sin.push_back(tmp);
    }
    
    getGreaterThanEqMatch(sfor, sin, ref);
    
    double tswc = 0;
    double tdt = 0;
    double tc = 0;
    double dca = 0;
    getResults(execs, trades, ref, tswc, tdt, tc, dca);
    
    cout<<"DollarCreditAvailable"<<","<<"NumberExecutions"<<","<<"TradedLotsWtCredit"<<","<<"TotalDeltaTraded"<<","<<"TotalComms"<<endl;
    cout<<dca<<","<<execs.size()<<","<<tswc<<","<<tdt<<","<<tc<<endl;
    return 0;
}
