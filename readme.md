复旦大学数字EDA课程2024春季学期项目-高层次综合

人员：林与正



运行方法为在YuHLS/目录下执行:

make

./YuHLS.exe benchmark/[case name].ll

测试用例放置于benchmark/目录下，make编译完成后生成YuHLS.exe，执行后生成output文件夹，其中放置结果[case name].v，并打印出执行信息日志。



benchmark-dotprod:

define int dotprod(int a[], int b[], int n)
    c = 0;

start:
    i = phi(0, 0, i_inc, calc);
    cond = i >= n;
    br cond ret calc

calc:
    ai = load(a, i);
    bi = load(b, i);
    cl = phi(c, 0, cr, start);
    ci = ai * bi;
    cr = cl + ci;
    i_inc = i + 1;
    br start;

ret:
    cf = phi(0, c, start, cr);
    return cf;

