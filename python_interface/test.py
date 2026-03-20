from fflow import *
from fflowfirefly import *


def test(idx):
    print("Testing Firefly reconstruction {}...".format(idx))

    myvars = ["x","y","z"]

    with GraphContextWithInput(len(myvars)) as (g,inp):

        funcs = ["1/2345673456783456784567*x+y^2","x^3+x y^3","x^5+z y^5"]
        node = AlgRatFunEval(g,inp,ParseRatFun(myvars,funcs))
        SetOutputNode(g,node)

        rec = FireflyReconstructFunction(g
                                         #,verbosity=FIREFLY_CHATTY
                                         )

        pt = [456789678,676767896789,67867809887777]
        prime_no = 4

        tofffun = RatExprToRatFunList(rec.to_string(myvars),myvars)
        evalpt = EvaluateRatFunList(tofffun,pt,prime_no)
        check = EvaluateRatFunList(ParseRatFun(myvars,funcs),pt,prime_no)

        if evalpt != check:
            print("- Test failed!")
        print("- Test passed!")


def test_failure():
    print("Testing a Firefly reconstruction that's supposed to fail...")

    myvars = ["x","y","z"]
    extra_vars = ["x","y","z","w"]

    with GraphContextWithInput(len(myvars)) as (g,inp):

        inp2 = AlgTake(g,[inp],[(0,0),(0,1),(0,2),(0,2)])

        # note we are dividing by zero
        funcs = ["1/2345673456783456784567*x+y^2","x^3+x y^3","(x^5+y^5)/(z-w)"]
        node = AlgRatFunEval(g,inp2,ParseRatFun(extra_vars,funcs))
        SetOutputNode(g,node)

        rec = FireflyReconstructFunction(g)
        if rec == FFlowError():
            print("- Test passed - i.e. \"failed successfully\" :)")
        else:
            print("- Test failed!")


if __name__ == '__main__':
    test(1)
    test(2)
    test_failure()
