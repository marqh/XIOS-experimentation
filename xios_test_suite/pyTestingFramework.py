import copy
import glob
import itertools
import json
import os
import subprocess
import unittest


# test case construction based on existence of folders
test_root = os.path.dirname(__file__)



def product_dict(**kwargs):
    keys = kwargs.keys()
    vals = kwargs.values()
    for instance in itertools.product(*vals):
        yield dict(zip(keys, instance))


# let's try one test case
# update iodef.xml from json config
# use

test_dirs = [os.path.join(test_root, 'TEST_SUITE', 'test_axis_algo')]

with open(os.path.join(test_root, 'TEST_SUITE', 'default_param.json'), 'r') as dparam:
    dparams = json.load(dparam)[0]

for atest_dir in test_dirs:
    with open(os.path.join(atest_dir, 'user_param_basic.json'), 'r') as uparam:
        config_list = []
        tmp_config_list = []
        config_dict = json.load(uparam)
        for i in range(len(config_dict)):
            tmp_config_list.extend(list(product_dict(**config_dict[i])))
        for config in tmp_config_list:
            new_config = copy.copy(dparams)
            new_config.update(config)
            config_list.append(new_config)
        # print(config_list)

    #  just the first element for now
    for config_dict in config_list[0:1]:

        with open(os.path.join(test_root, 'TEST_SUITE/iodef.xml'), 'r') as iodef:
            new_iodef = iodef.read()
            xpattern = 'XIOS::'
            # just take 1st one for now (until run established)

            for param in config_dict.items():
                new_iodef = new_iodef.replace(xpattern + param[0], str(param[1]))
            if xpattern in new_iodef:
                matching_lines = [line for line in new_iodef.split('\n') if xpattern in line]
                print(matching_lines)
                raise ValueError('params not updated: {}'.format('\n'.join(matching_lines)))
            with open(os.path.join(atest_dir, 'iodef.xml'), 'w') as test_iodef:
                test_iodef.write(new_iodef)

        with open(os.path.join(atest_dir, "param.def"), "w") as fh:
            fh.write("&params_run\n")
            fh.write("duration=\'"+config_dict["Duration"]+"\'\n")
            # fh.write("nb_proc_atm="+str(config_dict["NumberClients"])+"\n")
            # hold n clients at 1 whilst messaging issue persists
            fh.write("nb_proc_atm="+str(1)+"\n")
            fh.write("/\n")

        if not os.path.exists(os.path.join(atest_dir, 'context_grid_dynamico.xml')):
            os.symlink(os.path.join(test_root, 'TEST_SUITE', 'context_grid_dynamico.xml'),
                       os.path.join(atest_dir, 'context_grid_dynamico.xml'))
        if not os.path.exists(os.path.join(atest_dir, 'dynamico_grid.nc')):
            os.symlink(os.path.join(test_root, 'TEST_SUITE', 'dynamico_grid.nc'),
                       os.path.join(atest_dir, 'dynamico_grid.nc'))
        if not os.path.exists(os.path.join(atest_dir, 'generic_testcase.exe')):
            os.symlink(os.path.join((os.path.dirname(test_root)), 'bin', 'generic_testcase.exe'),
                       os.path.join(atest_dir, 'generic_testcase.exe'))

        os.chdir(atest_dir)
        acall = ['mpirun', '-np', '1', 'generic_testcase.exe']
        print(' '.join(acall))
        subprocess.check_call(acall)
