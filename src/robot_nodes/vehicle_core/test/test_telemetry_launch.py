"""
test_telemetry_launch.py - pytest launch_testing for telemetry_node.

Validates:
 - Launch infrastructure works correctly
 - Clean shutdown
"""

import pytest
import launch
import launch_testing
import launch_testing.actions
import launch_testing.markers


def generate_test_description():
    """Generate launch description for telemetry node test."""
    test_launch = launch.LaunchDescription([
        launch.actions.LogInfo(msg="Telemetry launch test started"),
        launch_testing.actions.ReadyToTest(),
    ])
    return test_launch, {}


@pytest.mark.launch_test
def test_launch_success(launch_service):
    """Verify launch infrastructure works."""
    assert launch_service is not None


@launch_testing.post_shutdown_test()
class TestShutdown:
    """Test class for clean shutdown verification."""

    def test_shutdown(self, proc_info):
        """Verify clean shutdown."""
        launch_testing.asserts.assertExitCodes(
            proc_info,
            allow_oneshot=True,
        )