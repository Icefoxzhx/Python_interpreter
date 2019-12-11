
#ifndef PYTHON_INTERPRETER_DATATYPE_H
#define PYTHON_INTERPRETER_DATATYPE_H

#include <string>
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

class BigInt;

class DataType;

map<string,DataType*>VAR[4010];

int CUR=0;

vector<DataType*>rec;

map<string, Python3Parser::FuncdefContext*>Func;
map<string, DataType* >Funcp;
class BigUInt:vector<int>{
public:
    friend BigInt;
    BigUInt(){}
    BigUInt(const string &s){
        for(auto it=s.rbegin();it!=s.rend();++it)
            this->push_back(*it-'0');
    }
    ~BigUInt(){}
    void check(){
        while(!this->empty()&&!this->back()) this->pop_back();
    }
    friend bool operator<(const BigUInt &x,const BigUInt &y){
        if(x.size()!=y.size()) return x.size()<y.size();
        for(int i=x.size()-1;i>=0;--i)
            if(x[i]!=y[i]) return x[i]<y[i];
        return false;
    }
    friend bool operator==(const BigUInt &x,const BigUInt &y){
        if(x.size()!=y.size()) return false;
        for(int i=x.size()-1;i>=0;--i)
            if(x[i]!=y[i]) return false;
        return true;
    }
    friend bool operator<=(const BigUInt &x,const BigUInt &y){
        return x<y||x==y;
    }
    friend BigUInt operator+(const BigUInt &x,const BigUInt &y){
        BigUInt res=x;
        res.resize(max(y.size(),x.size())+1);
        for(int i=0,n=y.size();i<n;++i){
            res[i]+=y[i];
            if(res[i]>=10) res[i]-=10,res[i+1]++;
        }
        int j=y.size();
        while(j<res.size()&&res[j]>=10) res[j]-=10,res[++j]++;
        res.check();
        return res;
    }
    friend BigUInt operator-(const BigUInt &x,const BigUInt &y){
        BigUInt res=x;
        for(int i=0,n=y.size();i<n;++i){
            res[i]-=y[i];
            if(res[i]<0) res[i]+=10,res[i+1]--;
        }
        int j=y.size();
        while(j<x.size()&&res[j]<0) res[j]+=10,res[++j]--;
        res.check();
        return res;
    }
    friend BigUInt operator*(const BigUInt &x,const BigUInt &y){
        BigUInt res;
        res.resize(x.size()+y.size()+10);
        for(int i=0,nx=x.size();i<nx;++i)
            for(int j=0,ny=y.size();j<ny;++j)
                res[i+j]+=x[i]*y[j];
        for(int i=0,n=res.size()-1;i<n;++i)
            res[i+1]+=res[i]/10,res[i]%=10;
        res.check();
        return res;
    }
    friend BigUInt operator/(const BigUInt &x,const BigUInt &y){//DIV
        BigUInt res,z=x;
        for(int i=z.size()-y.size();y<=z;--i){
            BigUInt c;c.assign(i+1,0);c.back()=1;
            BigUInt d=c*y;
            while(d<=z){
                z=z-d;
                res=res+c;
            }
        }
        res.check();
        return res;
    }
};
class BigInt{
public:
    int sgn;
    BigUInt a;
    BigInt(){sgn=0;}
    ~BigInt(){}
    void check(){
        a.check();if(a.empty()) sgn=0;
    }
    explicit BigInt(const string &s){
        if(s.find('-')!=s.npos) sgn=-1;
        else sgn=1;
        for(auto it=s.rbegin();it!=s.rend();++it)
            if(*it!='-') a.push_back(*it-'0');
        this->check();
    }
    explicit BigInt(const bool x){
        if(x) a.push_back(1),sgn=1;
        else sgn=0;
    }
    explicit operator string() const {
        string res;
        if(!sgn) res='0';
        if(sgn==-1) res='-';
        if(sgn) {
            for (auto it = a.rbegin(); it != a.rend(); ++it)
                res += std::to_string(*it );
        }
        return res;
    }
    explicit operator double() const{
        double res=0;
        for(auto it=a.rbegin();it!=a.rend();++it)
            res=res*10+*it;
        return res*sgn;
    }
    explicit operator bool() const{
        return !a.empty();
    }
    friend ostream &operator<<(ostream &os,const BigInt &x){
        os<<string(x);
        return os;
    }
    friend bool operator<(const BigInt &x,const BigInt &y){
        if(x.sgn!=y.sgn) return x.sgn<y.sgn;
        if(x.sgn==-1) return y.a<x.a;
        return x.a<y.a;
    }
    friend bool operator==(const BigInt &x,const BigInt &y){
        if(x.sgn!=y.sgn) return false;
        return x.a==y.a;
    }
    friend bool operator>(const BigInt &x,const BigInt &y){
        return !(x<y||x==y);
    }
    BigInt operator+(const BigInt &y){
        BigInt res;
        if(sgn*y.sgn!=1){
            res.sgn=a<y.a?y.sgn:sgn;
            res.a=a<y.a?y.a-a:a-y.a;
        }else {
            res.sgn = sgn;
            res.a = a + y.a;
        }
        if(res.a.empty()) res.sgn=0;
        return res;
    }
    BigInt operator-(const BigInt &y){
        BigInt res=y;
        res.sgn=-res.sgn;
        return *this+res;
    }
    friend BigInt operator*(const BigInt &x,const BigInt &y){
        BigInt res;
        res.sgn=x.sgn*y.sgn;
        res.a=x.a*y.a;
        return res;
    }
    BigInt operator/(const BigInt &y){
        BigInt res;
        res.sgn=sgn*y.sgn;
        res.a=a/y.a;
        if(res.sgn==-1&&!(res.a*y.a==a)) res.a=res.a+BigUInt(to_string(1));
        if(res.a.empty()) res.sgn=0;
        return res;
    }
};
enum Type{String,Int,Double,Bool,Null,Vector};
enum F_type{None,Break,Continue,Return};
class DataType{
public:
    F_type FT;
    Type T;
    string s;
    BigInt a;
    double b;
    bool c;
    vector<DataType> d;
    DataType(){FT=None;T=Null;}
    ~DataType(){}
    DataType(const string &x){
        FT=None;T=String;s=x;
    }
    DataType (const BigInt &x){
        FT=None;T=Int;a=x;
    }
    DataType(const double &x){
        FT=None;T=Double;b=x;
    }
    DataType(const bool &x){
        FT=None;T=Bool;c=x;
    }
    DataType &operator=(const string &x){
        T=String;s=x;
        return *this;
    }
    DataType &operator=(const BigInt &x){
        T=Int;a=x;
        return *this;
    }
    DataType &operator=(const double &x){
        T=Double;b=x;
        return *this;
    }
    DataType &operator=(const bool &x){
        T=Bool;c=x;
        return *this;
    }
    friend ostream &operator<<(ostream &os, const DataType &x){
        switch(x.T){
            case String: os<<x.s;break;
            case Int: os<<x.a;break;
            case Double: os<<fixed<<setprecision(6)<<x.b;break;
            case Bool: os<<(x.c?"True":"False");break;
            case Null:os<<"None";break;
        }
        return os;
    };
    friend DataType operator+(DataType x,DataType y){
        DataType res;
        if(x.T==Bool) x.toInt();
        if(y.T==Bool) y.toInt();
        if(x.T==String){res.s=x.s+y.s;res.T=String;return res;}
        if(x.T==Int){
            switch(y.T){
                case Int: res.a=x.a+y.a;res.T=Int;break;
                case Double: res.b=(double)x.a+y.b;res.T=Double;break;
            }
        }
        else{
            switch(y.T){
                case Int: res.b=x.b+(double)y.a;res.T=Double;break;
                case Double: res.b=x.b+y.b;res.T=Double;break;
            }
        }
        return res;
    }
    friend DataType operator-(DataType x,DataType y){
        DataType res;
        if(x.T==Bool) x.toInt();
        if(y.T==Bool) y.toInt();
        if(x.T==Int){
            switch(y.T){
                case Int: res.a=x.a-y.a;res.T=Int;break;
                case Double: res.b=(double)x.a-y.b;res.T=Double;break;
            }
        }
        else{
            switch(y.T){
                case Int: res.b=x.b-(double)y.a;res.T=Double;break;
                case Double: res.b=x.b-y.b;res.T=Double;break;
            }
        }
        return res;
    }
    friend DataType operator*(DataType x,DataType y){
        DataType res;
        if(x.T==Bool) x.toInt();
        if(y.T==Bool) y.toInt();
        if(x.T==String){
            int n=stoi((string)y.a);
            while(n--) res.s+=x.s;
            res.T=String;
            return res;
        }
        if(y.T==String){
            int n=stoi((string)x.a);
            while(n--) res.s+=y.s;
            res.T=String;
            return res;
        }
        if(x.T==Int){
            switch(y.T){
                case Int: res.a=x.a*y.a;res.T=Int;break;
                case Double: res.b=(double)x.a*y.b;res.T=Double;break;
            }
        }
        else{
            switch(y.T){
                case Int: res.b=x.b*(double)y.a;res.T=Double;break;
                case Double: res.b=x.b*y.b;res.T=Double;break;
            }
        }
        return res;
    }
    friend DataType operator/(DataType x,DataType y){//float
        DataType res;
        x.toDouble();
        y.toDouble();
        res.T=Double;
        res.b=x.b/y.b;
        return res;
    }
    friend DataType Div(DataType x,DataType y){
        DataType res;
        res.T=Int;
        res.a=x.a/y.a;
        return res;
    }
    friend DataType operator%(DataType x,DataType y){
        DataType res;
        res.T=Int;
        res.a=x.a-x.a/y.a*y.a;
        return res;
    }
    friend bool operator<(DataType x,DataType y){
        if(x.T==Bool) x.toInt();
        if(y.T==Bool) y.toInt();
        if(x.T==String) return x.s<y.s;
        if(x.T==Int){
            if(y.T==Int) return x.a<y.a;
            else return (double)x.a<y.b;
        }else{
            if(y.T==Int) return x.b<(double)y.T;
            else return x.b<y.b;
        }
    }
    friend bool operator>(DataType x,DataType y){
        if(x.T==Bool) x.toInt();
        if(y.T==Bool) y.toInt();
        if(x.T==String) return x.s>y.s;
        if(x.T==Int){
            if(y.T==Int) return x.a>y.a;
            else return (double)x.a>y.b;
        }else{
            if(y.T==Int) return x.b>(double)y.a;
            else return x.b>y.b;
        }
    }
    friend bool operator==(DataType x,DataType y){
        if(x.T==Bool) x.toInt();
        if(y.T==Bool) y.toInt();
        if(x.T==String) return x.s==y.s;
        if(x.T==Int){
            if(y.T==Int) return x.a==y.a;
            else return (double)x.a==y.b;
        }else{
            if(y.T==Int) return x.b==(double)y.a;
            else return x.b==y.b;
        }
    }
    friend bool operator!=(const DataType& x,const DataType& y){
        return !(x==y);
    }
    friend bool operator>=(const DataType& x,const DataType& y){
        return x>y||x==y;
    }
    friend bool operator<=(const DataType& x,const DataType& y){
        return x<y||x==y;
    }
    void toInt(){
        switch(T){
            case String: a=BigInt(s);break;
            case Double: a=BigInt(to_string((long long)b));break;
            case Bool: s=c?'1':'0';a=BigInt(s);break;
        }
        T=Int;
    }
    void toDouble(){
        switch(T){
            case Int: b=(double)a;break;
            case Bool: b=c;break;
        }
        T=Double;
    }
    void toString(){
        switch(T){
            case Int: s=string(a);break;
            case Double: s=to_string(b);break;
            case Bool: s=c?"True":"False";break;
            case Null: s="None";break;
        }
        T=String;
    }
    void toBool(){
        switch(T){
            case Int: c=(bool)a;break;
            case Double: c=b;break;
            case String: c=!s.empty();break;
            case Null: c=false;break;
        }
        T=Bool;
    }
};
#endif //PYTHON_INTERPRETER_DATATYPE_H
