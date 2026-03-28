import argparse
import os
import subprocess
import yaml
import json
import datetime
import re
import platform

# 7-bit C1 ANSI sequences
ansi_escape = re.compile(r'''
    \x1B  # ESC
    (?:   # 7-bit C1 Fe (except CSI)
        [@-Z\\-_]
    |     # or [ for CSI, followed by a control sequence
        \[
        [0-?]*  # Parameter bytes
        [ -/]*  # Intermediate bytes
        [@-~]   # Final byte
    )
''', re.VERBOSE)

def main():
    # Obtain the cmd line arguments
    parser = argparse.ArgumentParser(description="WIMP test")
    parser.add_argument("--test-names", type=str, help="Specify test names to run, separated by commas")
    parser.add_argument("--executable-root", type=str, help="Specify the root directory for test executables")
    parser.add_argument("--tests-root", type=str, help="Specify the root directory for test run files")
    parser.add_argument("--report-file", type=str, help="Specify the file to write the report to")
    parser.add_argument("--compile-mode", type=str, choices=["debug", "release"], default="release", help="Specify the compile mode for the tests (default: release)")
    args = parser.parse_args()

    # Get the test root directory. If not provided, throw an error
    if not args.executable_root:
        print("Error: --executable-root argument is required.")
        return 1
    
    executable_root = args.executable_root

    # Get the test run files root directory. If not provided, throw an error
    if not args.tests_root:
        print("Error: --tests-root argument is required.")
        return 1
    
    tests_root = args.tests_root

    # Get the test names. If not provided, run all tests in the test root directory
    test_names = []
    if args.test_names:
        test_names = args.test_names.split(",")
        # Check if the specified test names exist in the test root directory
        for test_file_name in test_names:
            test_path = os.path.join(tests_root, test_file_name)
            if not os.path.isfile(test_path):
                print(f"Error: Test {test_file_name} not found in the root directory")
                return 2
    else:
        # Create a list of all the test files in the test root directory
        for file in os.listdir(tests_root):
            # Only include files with a ".yml" extension
            if file.endswith(".yml"):
                test_names.append(file)
    
    if len(test_names) == 0:
        print("No tests found matching the criteria in the root directory")
        return 0

    # Print the tests to run
    print(f"Running the following tests: {test_names}")

    # Create the final report dictionary
    report = {
        "results": 
        {
            "tool": {
            "name": "wimp-test"
            },
            "summary": {
            "tests": len(test_names),
            "passed": 0,
            "failed": 0,
            "skipped": 0,
            "warnings": 0,
            "start": 0,
            "stop": 0
            },
            "tests": [],
            "environment": {
            "appName": "WIMP",
            "buildName": f"WIMP ({args.compile_mode})",
            "buildNumber": "0.3.0",
            }
        }
    }

    # Run the tests for each
    start_time = datetime.datetime.now()
    for test_file_name in test_names:
        # Load the test run file
        test_path = os.path.join(tests_root, test_file_name)
        with open(test_path, 'r') as f:
            data = yaml.load(f, Loader=yaml.SafeLoader)

        test_name = data.get("name", "Unnamed Test")
        test_suite = data.get("suite", "Unnamed Suite")
        print(f"Running test: {test_name} from suite: {test_suite}")

        # Run the base test
        test_start = datetime.datetime.now()
        base_test = os.path.join(executable_root, data.get("base_test"))
        # If windows append .exe to the executable root if not already present
        if platform.system() == "Windows" and not base_test.endswith(".exe"):
            base_test += ".exe"
        test_duration = (datetime.datetime.now() - test_start).total_seconds()

        if not os.path.isfile(base_test):
            print(f"Error: Base test executable {base_test} not found")
            report["results"]["summary"]["skipped"] += 1
            continue

        result = subprocess.run([base_test], capture_output=True, text=True, cwd=executable_root)
        output_lines = result.stdout.splitlines()

        # Parse the output and check for success
        overall_result = "failed"        

        # Perform return code check
        expected_return_code = data.get("expected_return_code", 0)
        if result.returncode != expected_return_code:
            # This is part of the fail
            overall_result = "failed"
            print(f"Fail condition met: Return code was {result.returncode}, expected {expected_return_code}")

        # Perform pass conditions check (all pass conditions must be met for the test to pass)
        pass_conditions = data.get("pass_conditions", [])
        for condition in pass_conditions:
            if not any(condition in line for line in output_lines):
                overall_result = "failed"
                # Print the fail line not just the condition
                print(f"Pass condition not met: {condition}")
            else:
                overall_result = "passed"

        # Perform fail conditions check
        fail_conditions = data.get("fail_conditions", [])
        for condition in fail_conditions:
            if any(condition in line for line in output_lines):
                overall_result = "failed"
                # Print the fail line not just the condition
                for line in output_lines:
                    if condition in line:
                        # Remove any ANSI escape codes from the line before printing
                        print(f"Fail condition met: {ansi_escape.sub('', line)}")

        # Perform warnings check
        warning_conditions = data.get("warning_conditions", [])
        for condition in warning_conditions:
            if any(condition in line for line in output_lines):
                # Print the warning line not just the condition
                for line in output_lines:
                    if condition in line:
                        # Remove any ANSI escape codes from the line before printing
                        print(f"Warning condition met: {ansi_escape.sub('', line)}")

                report["results"]["summary"]["warnings"] += 1
        
        print(f"{test_suite}/{test_name} result: {overall_result}")

        current_platform = platform.system()
        test = {
            "name": f"{current_platform}/{test_suite}/{test_name}",
            "status": overall_result,
            "duration": test_duration,
            "suite": f"{current_platform}/{test_suite}",
        }
        report["results"]["tests"].append(test)
        if overall_result == "passed":
            report["results"]["summary"]["passed"] += 1
        elif overall_result == "failed":
            report["results"]["summary"]["failed"] += 1
        elif overall_result == "skipped":
            report["results"]["summary"]["skipped"] += 1
        else:
            report["results"]["summary"]["other"] += 1
    
    stop_time = datetime.datetime.now()
    report["results"]["summary"]["start"] = start_time.isoformat()
    report["results"]["summary"]["stop"] = stop_time.isoformat()
    
    # Print the final report as JSON
    report_file_name = f"report_{test_suite}_{start_time.year}-{start_time.month}-{start_time.day}-{start_time.hour}-{start_time.minute}-{start_time.second}.json"
    if args.report_file:
        report_file_name = args.report_file
    json.dump(report, open(report_file_name, "w"), indent=4)

    return report["results"]["summary"]["failed"]

if __name__ == "__main__":
    main()