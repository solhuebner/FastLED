"""Tests for github_project_sync.py config parsing and status logic."""

from __future__ import annotations

import os
import unittest
from unittest.mock import patch

from ci.github_project_sync import Config, determine_status


class TestConfigFromEnv(unittest.TestCase):
    """Test Config.from_env() environment variable parsing."""

    BASE_ENV = {
        "PROJECT_OWNER": "FastLED",
        "PROJECT_OWNER_TYPE": "organization",
        "PROJECT_NUMBER": "1",
        "STATUS_FIELD": "Status",
        "STATUS_TODO": "Todo",
        "STATUS_DONE": "Done",
        "STATUS_DRAFT": "Draft",
        "DATE_FIELD": "Date posted",
        "EVENT_NAME": "issues",
        "EVENT_ACTION": "opened",
        "ITEM_NODE_ID": "I_abc123",
        "IS_DRAFT": "false",
    }

    @patch.dict(os.environ, BASE_ENV, clear=False)
    def test_parses_all_fields(self) -> None:
        config = Config.from_env()
        self.assertEqual(config.project_owner, "FastLED")
        self.assertEqual(config.project_owner_type, "organization")
        self.assertEqual(config.project_number, 1)
        self.assertEqual(config.status_field, "Status")
        self.assertEqual(config.status_todo, "Todo")
        self.assertEqual(config.status_done, "Done")
        self.assertEqual(config.status_draft, "Draft")
        self.assertEqual(config.date_field, "Date posted")
        self.assertEqual(config.item_node_id, "I_abc123")
        self.assertFalse(config.is_draft)

    @patch.dict(os.environ, {**BASE_ENV, "IS_DRAFT": "true"}, clear=False)
    def test_is_draft_true(self) -> None:
        config = Config.from_env()
        self.assertTrue(config.is_draft)

    @patch.dict(os.environ, {**BASE_ENV, "STATUS_DRAFT": ""}, clear=False)
    def test_empty_draft_status(self) -> None:
        config = Config.from_env()
        self.assertEqual(config.status_draft, "")

    @patch.dict(os.environ, {**BASE_ENV, "DATE_FIELD": ""}, clear=False)
    def test_empty_date_field(self) -> None:
        config = Config.from_env()
        self.assertEqual(config.date_field, "")

    @patch.dict(
        os.environ,
        {k: v for k, v in BASE_ENV.items() if k != "PROJECT_NUMBER"},
        clear=False,
    )
    def test_missing_project_number_exits(self) -> None:
        os.environ.pop("PROJECT_NUMBER", None)
        with self.assertRaises(SystemExit):
            Config.from_env()

    @patch.dict(
        os.environ,
        {k: v for k, v in BASE_ENV.items() if k != "ITEM_NODE_ID"},
        clear=False,
    )
    def test_missing_item_node_id_exits(self) -> None:
        os.environ.pop("ITEM_NODE_ID", None)
        with self.assertRaises(SystemExit):
            Config.from_env()


class TestDetermineStatus(unittest.TestCase):
    """Test status determination logic."""

    def _make_config(self, **overrides) -> Config:
        defaults = {
            "project_owner": "FastLED",
            "project_owner_type": "organization",
            "project_number": 1,
            "status_field": "Status",
            "status_todo": "Triage",
            "status_done": "Done",
            "status_draft": "In Progress",
            "date_field": "",
            "event_name": "issues",
            "event_action": "opened",
            "item_node_id": "I_abc",
            "is_draft": False,
        }
        defaults.update(overrides)
        return Config(**defaults)

    def test_closed_returns_done(self) -> None:
        config = self._make_config(event_action="closed")
        self.assertEqual(determine_status(config), "Done")

    def test_opened_issue_returns_triage(self) -> None:
        config = self._make_config(event_name="issues", event_action="opened")
        self.assertEqual(determine_status(config), "Triage")

    def test_opened_pr_returns_triage(self) -> None:
        config = self._make_config(
            event_name="pull_request_target", event_action="opened", is_draft=False
        )
        self.assertEqual(determine_status(config), "Triage")

    def test_draft_pr_returns_in_progress(self) -> None:
        config = self._make_config(
            event_name="pull_request_target",
            event_action="opened",
            is_draft=True,
            status_draft="In Progress",
        )
        self.assertEqual(determine_status(config), "In Progress")

    def test_draft_pr_without_draft_status_returns_triage(self) -> None:
        config = self._make_config(
            event_name="pull_request_target",
            event_action="opened",
            is_draft=True,
            status_draft="",
        )
        self.assertEqual(determine_status(config), "Triage")

    def test_reopened_returns_triage(self) -> None:
        config = self._make_config(event_action="reopened")
        self.assertEqual(determine_status(config), "Triage")

    def test_ready_for_review_returns_triage(self) -> None:
        config = self._make_config(
            event_name="pull_request_target",
            event_action="ready_for_review",
            is_draft=False,
        )
        self.assertEqual(determine_status(config), "Triage")


if __name__ == "__main__":
    unittest.main()
