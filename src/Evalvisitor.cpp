#include "Evalvisitor.h"
#include "DataType.h"

antlrcpp::Any EvalVisitor::visitFile_input(Python3Parser::File_inputContext *ctx) {
	visitChildren(ctx);
	for(auto & it : rec)
	    delete it;
	return 0;
}

antlrcpp::Any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx) {
	string s=ctx->NAME()->getText();
	Func[s]=ctx;
	rec.push_back(Funcp[s]=new DataType);
	DataType *p=Funcp[s];
	if(ctx->parameters()->typedargslist()){
	    for(size_t i=0,n=ctx->parameters()->typedargslist()->test().size();i<n;++i){
	        p->d.push_back(visit(ctx->parameters()->typedargslist()->test(i)));
	        p->d[i]=p->d[i].rval();
	    }
	}
	return DataType();
}

antlrcpp::Any EvalVisitor::visitParameters(Python3Parser::ParametersContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitTfpdef(Python3Parser::TfpdefContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx) {
    if (ctx->simple_stmt()) {
        return visit(ctx->simple_stmt());
    } else {
        return visit(ctx->compound_stmt());
    }
}

antlrcpp::Any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
	return visit(ctx->small_stmt());
}

antlrcpp::Any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) {
	if(ctx->expr_stmt()) return visit(ctx->expr_stmt());
	return visit(ctx->flow_stmt());
}

antlrcpp::Any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
    if(ctx->augassign()){
        DataType b=visit(ctx->testlist().back()->test(0));
        DataType a=visit(ctx->testlist(0)->test(0));
        string s=a.s;
        assert(VAR[CUR][s]);
        if(ctx->augassign()->ADD_ASSIGN()) *VAR[CUR][s]=a+b;
        else if(ctx->augassign()->SUB_ASSIGN()) *VAR[CUR][s]=a-b;
        else if(ctx->augassign()->DIV_ASSIGN()) *VAR[CUR][s]=a/b;
        else if(ctx->augassign()->IDIV_ASSIGN()) *VAR[CUR][s]=Div(a,b);
        else if(ctx->augassign()->MOD_ASSIGN()) *VAR[CUR][s]=a%b;
        else if(ctx->augassign()->MULT_ASSIGN()) *VAR[CUR][s]=a*b;
        return *VAR[CUR][s];
    }else if(!ctx->ASSIGN().empty()){
        vector<DataType>b=visit(ctx->testlist().back());
        for(int i=(int)ctx->testlist().size()-2;i>=0;--i){
            for(size_t j=0,m=b.size();j<m;++j){
                string s=visit(ctx->testlist(i)->test(j)).as<DataType>().s;
                if(!VAR[CUR][s]){
                    rec.push_back(VAR[CUR][s]=new DataType);
                }
                *VAR[CUR][s]=b[j];
            }
        }
        return DataType();
    }
    return visit(ctx->testlist(0)->test(0));
}

antlrcpp::Any EvalVisitor::visitAugassign(Python3Parser::AugassignContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) {
    DataType res;
	if(ctx->break_stmt()){res.FT=Break;return res;}
	if(ctx->continue_stmt()){res.FT=Continue;return res;}
	res=visit(ctx->return_stmt()).as<DataType>();res.FT=Return;return res;
}

antlrcpp::Any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) {
    if (ctx->testlist()) {
        if(ctx->testlist()->test().size()==1){
            DataType res=visit(ctx->testlist()->test().back());
            return res.rval();
        }
        DataType res;
        res.T=Vector;
        res.d=visit(ctx->testlist()).as<vector<DataType> >();
        return res;
    } else {
        return DataType();
    }
}

antlrcpp::Any EvalVisitor::visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) {
	if(ctx->funcdef()) return visit(ctx->funcdef());
	if(ctx->if_stmt()) return visit(ctx->if_stmt());
	return visit(ctx->while_stmt());
}

antlrcpp::Any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx) {
    for(size_t i=0,n=ctx->test().size();i<n;++i){
        DataType res=visit(ctx->test(i));
        res=res.rval();
        res.toBool();
        if(!res.c) continue;
        VAR[CUR+1]=VAR[CUR];++CUR;
        res=visit(ctx->suite(i)).as<DataType>();--CUR;
        return res;
    }
    if(ctx->ELSE()){
        VAR[CUR+1]=VAR[CUR];++CUR;
        DataType res=visit(ctx->suite().back());--CUR;
        return res;
    }
	return DataType();
}

antlrcpp::Any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
	while(true){
	    DataType res=visit(ctx->test());
	    res=res.rval();res.toBool();
	    if(!res.c) break;
        VAR[CUR+1]=VAR[CUR];++CUR;
	    res=visit(ctx->suite()).as<DataType>();--CUR;
	    if(res.FT==Break) break;
	    if(res.FT==Return) return res;
	}
	return DataType();
}

antlrcpp::Any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx) {
	if(ctx->simple_stmt()) return visit(ctx->simple_stmt());
	for(size_t i=0,n=ctx->stmt().size();i<n;++i){
	    DataType res=visit(ctx->stmt(i));
	    if(res.FT==Continue) return res;
	    if(res.FT==Break) return res;
	    if(res.FT==Return) return res;
	}
	return DataType();
}

antlrcpp::Any EvalVisitor::visitTest(Python3Parser::TestContext *ctx) {
	return visit(ctx->or_test());
}

antlrcpp::Any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
	if(ctx->OR().empty()){
		return visit(ctx->and_test(0));
	}else{
		auto a=ctx->and_test();
		for(auto & it : a) {
		    DataType res=visit(it);
		    res=res.rval();
		    res.toBool();
            if(res.c) return res;
        }
		return DataType(false);
	}
}

antlrcpp::Any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
	if(ctx->AND().empty()){
		return visit(ctx->not_test(0));
	}else{
		auto a=ctx->not_test();
		for(auto & it : a){
		    DataType res=visit(it);
		    res=res.rval();
		    res.toBool();
		    if(!res.c) return res;
		}
		return DataType(true);
	}
}

antlrcpp::Any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
	if(ctx->NOT()){
        DataType res=visitNot_test(ctx->not_test()).as<DataType>().rval();
        res.toBool();
        res.c=!res.c;
	    return res;
	}
	return visit(ctx->comparison());
}

antlrcpp::Any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
    if(ctx->comp_op().empty()) return visit(ctx->arith_expr(0));
    bool res=true;
    DataType lst=visit(ctx->arith_expr(0));
    for(size_t i=0,n=ctx->comp_op().size();i<n;++i){
        DataType x=visit(ctx->arith_expr(i+1));
        if(ctx->comp_op(i)->EQUALS()) res&=lst==x;
        else if(ctx->comp_op(i)->GREATER_THAN()) res&=lst>x;
        else if(ctx->comp_op(i)->LESS_THAN()) res&=lst<x;
        else if(ctx->comp_op(i)->GT_EQ()) res&=lst>=x;
        else if(ctx->comp_op(i)->LT_EQ()) res&=lst<=x;
        else res&=lst!=x;
        if(!res) return DataType(res);
        lst=x;
    }
    return DataType(res);
}

antlrcpp::Any EvalVisitor::visitComp_op(Python3Parser::Comp_opContext *ctx) {
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
	DataType res=visit(ctx->term(0));
	if(ctx->addsub_op().empty()) return res;
	for(size_t i=0,n=ctx->addsub_op().size();i<n;++i){
        if(ctx->addsub_op(i)->ADD()) res=res+visit(ctx->term(i+1)).as<DataType>();
        else res=res-visit(ctx->term(i+1)).as<DataType>();
	}
	return res;
}
antlrcpp::Any EvalVisitor::visitAddsub_op(Python3Parser::Addsub_opContext *ctx) {
    return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
	DataType res=visit(ctx->factor(0));
	if(ctx->muls_op().empty()) return res;
	for(size_t i=0,n=ctx->muls_op().size();i<n;++i){
	    if(ctx->muls_op(i)->STAR()) res=res*visit(ctx->factor(i+1)).as<DataType>();
	    else if(ctx->muls_op(i)->DIV()) res=res/visit(ctx->factor(i+1)).as<DataType>();
	    else if(ctx->muls_op(i)->MOD()) res=res%visit(ctx->factor(i+1)).as<DataType>();
	    else res=Div(res,visit(ctx->factor(i+1)).as<DataType>());
	}
	return res;
}

antlrcpp::Any EvalVisitor::visitMuls_op(Python3Parser::Muls_opContext *ctx) {
    return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
    if(ctx->addsub_op()){
        DataType res=visit(ctx->factor());
        if(ctx->addsub_op()->MINUS())
            res=res*DataType(BigInt((string)"-1"));
        return res;
    }
    return visit(ctx->atom_expr());
}

antlrcpp::Any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
    if(!ctx->trailer()) return visit(ctx->atom());
    string func=ctx->atom()->NAME()->getText();
    vector<DataType>a;
    vector<Python3Parser::ArgumentContext *>b;
    if(ctx->trailer()->arglist()){
        b=ctx->trailer()->arglist()->argument();
        for(size_t i=0,n=b.size();i<n;++i){
            a.push_back(visit(b[i]->test()));
            a[i]=a[i].rval();
        }
    }
    if(func== "print"){
        for(auto it=a.begin();it!=a.end();++it){
            if(it!=a.begin()) cout<<' ';
            cout<<*it;
        }
        puts("");
        return DataType();
    }
    if(func=="int"){
        auto res=a.back();
        res.toInt();
        return res;
    }
    if(func=="float"){
        auto res=a.back();
        res.toDouble();
        return res;
    }
    if(func=="str"){
        auto res=a.back();
        res.toString();
        return res;
    }
    if(func=="bool"){
        auto res=a.back();
        res.toBool();
        return res;
    }
    NEWVAR.clear();
    auto p=Func[func]->parameters()->typedargslist();
    auto c=Funcp[func]->d;
    if(p){
        for(size_t i=0,n=p->tfpdef().size(),m=c.size();i<n;++i){
            string s=p->tfpdef(i)->getText();
            rec.push_back(NEWVAR[s]=new DataType);
            if(i>=n-m) *NEWVAR[s]=c[m-n+i];
        }
        for(size_t i=0,n=a.size();i<n;++i){
            string s=b[i]->NAME()?b[i]->NAME()->getText():p->tfpdef(i)->getText();
            *NEWVAR[s]=a[i];
        }
    }
    VAR[++CUR]=VAR[0];
    for(auto &it:NEWVAR){
        VAR[CUR][it.first]=it.second;
    }
    DataType res=visit(Func[func]->suite());
    res.FT=None;
    --CUR;
    return res;
}

antlrcpp::Any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx) {
    if(ctx->arglist()) return visit(ctx->arglist());
	return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
    if(ctx->test()) return visit(ctx->test());
    DataType res;
    res.s=ctx->getText();res.T=String;
	if(!ctx->STRING().empty()){
	    string s;
	    auto a=ctx->STRING();
	    for(auto & it : a){
	        string x=it->getText();
	        x.erase(x.begin());
	        x.pop_back();
	        s+=x;
	    }
	    res.s=s;res.T=String;
	}
	if(ctx->NUMBER()){
	    string s=ctx->NUMBER()->getText();
	    if(s.find('.')!=s.npos) res.b=stod(s),res.T=Double;
	    else res.toInt();
	}
	if(ctx->NAME()) res.T=Var;
	if(ctx->NONE()) res.T=Null;
	if(ctx->FALSE()) res.T=Bool,res.c=false;
	if(ctx->TRUE()) res.T=Bool,res.c=true;
	return res;
}

antlrcpp::Any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
	auto a=ctx->test();
	vector<DataType>b;
    for(auto & it : a){
        b.push_back(visit(it));
        b.back()=b.back().rval();
    }
    if(b.back().T==Vector) b=b.back().d;
    return b;
}

antlrcpp::Any EvalVisitor::visitArglist(Python3Parser::ArglistContext *ctx) {
	return ctx->argument();
}

antlrcpp::Any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx) {
	return visit(ctx->test());
}
